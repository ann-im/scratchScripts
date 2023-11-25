import numpy as np
import pandas as pd

def calculate_angle(p1, p2, p3):
    """
    Calculate the angle at p2 formed by the line segments p1-p2 and p2-p3.
    The points are in the format [latitude, longitude].
    """
    # Convert to radians
    p1 = np.radians(p1)
    p2 = np.radians(p2)
    p3 = np.radians(p3)

    # Calculate the differences in the coordinates
    delta_p1_p2 = p2 - p1
    delta_p2_p3 = p3 - p2

    # Check for zero differences to avoid division by zero
    if np.isclose(delta_p1_p2[0], 0.0) or np.isclose(delta_p2_p3[0], 0.0):
        return 0  # Return a straight line angle if there's no significant change in latitude

    # Calculate the tangents
    tan1 = np.tan(delta_p1_p2[1]) / np.tan(delta_p1_p2[0])
    tan2 = np.tan(delta_p2_p3[1]) / np.tan(delta_p2_p3[0])

    # Calculate the angle in radians
    angle = np.arctan(np.abs((tan2 - tan1) / (1 + tan1 * tan2)))

    # Convert to degrees
    angle = np.degrees(angle)

    return angle

def filter_coordinates(GPSarray, threshold_angle):
    """
    Filters the GPS coordinates, removing points with an angle below the threshold.
    """
    # Convert the GPS array to a numpy array for easier calculations
    GPSarray = np.array(GPSarray)
    
    # This will store the indices of the points to keep
    keep_indices = []
    prevPoints = [[], []]
    prevIndices = [0, 0]
    for i in range(1, len(GPSarray) - 1):

        if len(prevPoints[0]) == 0:
            prevPoints[0] = GPSarray[i]
            prevIndices[0] = i
        else:
            if len(prevPoints[1]) != 0:
            # Calculate the angle between the three points
                angle = calculate_angle(prevPoints[0], prevPoints[1], GPSarray[i])
                print(f"Angle between points {prevIndices[0]}, {prevIndices[1]}, {i}: {angle}")

                # If the angle is less than or equal to WAYPOINT_ANGLE_MAX, store position
                #  Otherwise, replace the most recent point with the current position
                if angle >= threshold_angle:
                    prevPoints[0] = prevPoints[1]
                    prevIndices[0] = prevIndices[1]
                    keep_indices.append(i)

            prevPoints[1] = GPSarray[i]
            prevIndices[1] = i

    # Always keep the first and last points
    keep_indices = [0] + keep_indices + [len(GPSarray) - 1]

    # Filter the array
    filtered_GPSarray = GPSarray[keep_indices]

    return filtered_GPSarray.tolist()

# Load the uploaded CSV file
file_path = '/mnt/c/Users/sudom/Documents/GitHub/scratchScripts/logs/gpsLog_20231118_14-37-32.csv'
gps_data = pd.read_csv(file_path)

# Correcting the column names if necessary
gps_data.columns = gps_data.columns.str.strip()

# Extracting only the latitude and longitude columns
gps_array = gps_data[['latitude', 'longitude']].values.tolist()

print("Original number of points:", len(gps_array))

# Set your desired threshold angle here
threshold_angle = 15  # Example threshold angle in degrees
print("Threshold angle:", threshold_angle)

# Filter the GPS coordinates
filtered_GPSarray = filter_coordinates(gps_array, threshold_angle)

# Output or further processing of filtered_GPSarray can be done here
print("Number of points after filtering:", len(filtered_GPSarray))
for point in filtered_GPSarray:
    print(str(point).strip("[]"))
