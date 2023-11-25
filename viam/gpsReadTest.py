import csv

def extract_coordinates_from_csv(file_path):
    """
    Reads a CSV file with columns: timestamp, latitude, longitude, altitude.
    Extracts the latitude and longitude points and returns them in an array
    where each element is an array containing a pair of coordinates.

    Args:
    file_path (str): The path to the CSV file.

    Returns:
    list of lists: A list of [latitude, longitude] pairs.
    """
    coordinates = []

    with open(file_path, 'r') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)  # Skip the header row
        for row in csv_reader:
            if len(row) >= 3:
                latitude = float(row[1])
                longitude = float(row[2])
                coordinates.append([latitude, longitude])

    return coordinates

# Example usage:
coordinates = extract_coordinates_from_csv('/mnt/c/Users/oneof/Documents/GitHub/scratchScripts/logs/gpsLog_20231125_14-25-16.csv')
print(coordinates)
