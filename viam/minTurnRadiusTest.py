import math

def degrees_to_radians(degree):
    """Converts angular velocity from degrees to radians."""
    return degree * (math.pi / 180)

def radians_to_degrees(radian):
    """Converts angular velocity from radians to degrees."""
    return radian * (180 / math.pi)

def enforce_min_turn_radius(v, omega, min_radius):
    """
    Adjusts the linear velocity to maintain a minimum turn radius, keeping angular velocity constant.
    
    Parameters:
    v - original linear velocity in m/s
    omega - angular velocity in degrees/s (constant)
    min_radius - minimum allowable turn radius in meters
    
    Returns:
    adjusted_v - adjusted linear velocity in m/s
    """
    # Convert omega from degrees/s to radians/s for calculation
    omega_rad = degrees_to_radians(omega)

    # If the robot is moving straight or the original angular velocity is very small, return the original linear velocity.
    if abs(omega_rad) < 1e-6:
        return v

    # Calculate the turn radius with original velocities
    current_radius = v / omega_rad

    # Check if the current turn radius is smaller than the minimum radius
    if abs(current_radius) < min_radius:
        # Adjust linear velocity to enforce the minimum turn radius
        adjusted_v = omega_rad * min_radius
    else:
        # No adjustment needed
        return v

    return adjusted_v

def calculate_turn_radius(v, omega):
    """
    Calculates the effective turn radius from linear and angular velocities.

    Parameters:
    v - linear velocity in m/s
    omega - angular velocity in degrees/s

    Returns:
    turn_radius - effective turn radius in meters. Returns 'inf' for straight line movement.
    """
    # Convert omega from degrees/s to radians/s for calculation
    omega_rad = degrees_to_radians(omega)

    # Check if angular velocity is zero (straight line movement)
    if abs(omega_rad) < 1e-6:
        return float('inf')

    # Calculate turn radius
    turn_radius = v / omega_rad

    return turn_radius

import math

def differential_drive(forward, left):
    """
    Calculates the motor velocities for a differential drive robot.
    
    Parameters:
    forward - the forward velocity component
    left - the left velocity component
    
    Returns:
    (left_motor, right_motor) - velocities for the left and right motors
    """
    if forward < 0:
        # Mirror the forward turning arc if we go in reverse
        left_motor, right_motor = differential_drive(-forward, left)
        return -left_motor, -right_motor

    # convert to polar coordinates
    r = math.hypot(forward, left)
    t = math.atan2(left, forward)

    # rotate by 45 degrees
    t += math.pi / 4
    if t == 0:
        # Fix for a corner case in ATAN2
        t += 1.224647e-16 / 2

    # convert to cartesian
    left_motor = r * math.cos(t)
    right_motor = r * math.sin(t)

    # rescale the new coords
    left_motor *= math.sqrt(2)
    right_motor *= math.sqrt(2)

    # clamp to -1/+1
    left_motor = max(-1, min(left_motor, 1))
    right_motor = max(-1, min(right_motor, 1))

    return left_motor, right_motor

def enforce_min_turn_radius_fraction(linear, angular, min_radius, top_speed):
    """
    Adjusts the velocities to maintain a minimum turn radius.
    Limits linear velocity to 1 (100%) and adjusts angular velocity to compensate if necessary.
    
    Parameters:
    linear - original linear velocity as a fraction of top speed
    angular - angular velocity as a fraction of top speed
    min_radius - minimum allowable turn radius
    top_speed - the top speed of the robot
    
    Returns:
    adjusted_linear - adjusted linear velocity as a fraction of top speed
    adjusted_angular - adjusted angular velocity as a fraction of top speed
    """
    # Check if angular velocity is zero (straight line movement)
    if abs(angular) < 1e-6:
        return linear, angular
    
    # Limit power to the greater of the two inputs
    max_command = max(abs(linear), abs(angular))
    effective_top_speed = top_speed * max_command

    # Convert to actual velocities
    v = linear * top_speed
    omega = angular * top_speed

    # Convert omega to radians/s (assuming 1m radius for 1m/s linear speed as base for conversion)
    omega_rad = omega / 1.0
    
    # Calculate the turn radius with original velocities
    current_radius = v / omega_rad

    # Adjust linear or angular velocity based on the minimum turn radius
    if abs(current_radius) < min_radius:
        # Adjust linear velocity
        adjusted_v = omega_rad * min_radius
        adjusted_linear = max(min(adjusted_v / effective_top_speed, max_command), -1*max_command)

        # Adjust angular velocity if linear velocity is capped at 1 (100%)
        if abs(adjusted_linear) >= max_command:
            # Use the capped linear velocity to calculate adjusted angular velocity
            capped_v = effective_top_speed
            adjusted_omega_rad = capped_v / min_radius
            adjusted_angular = adjusted_omega_rad / top_speed
        else:
            adjusted_angular = angular

        # Linear should keep sign to preserve overall direction
        adjusted_linear = math.copysign(adjusted_linear, linear)
        adjusted_angular = math.copysign(adjusted_angular, angular)
    else:
        # No adjustment needed
        return linear, angular

    return adjusted_linear, adjusted_angular

def calculate_turn_radius_percent(linear_percent, angular_percent, top_speed):
    """
    Calculates the effective turn radius from linear and angular velocities given as percentages of top speed.
    
    Parameters:
    linear_percent - linear velocity as a percentage of top speed
    angular_percent - angular velocity as a percentage of top speed
    top_speed - the top speed of the robot in meters per second
    
    Returns:
    turn_radius - effective turn radius in meters. Returns 'inf' for straight line movement.
    """
    # Convert percentages to actual velocities
    v = linear_percent / 100.0 * top_speed
    # Assuming angular velocity at 100% corresponds to top speed at 1m radius
    omega = angular_percent / 100.0 * top_speed

    # Convert omega to radians/s (assuming 1m radius for 1m/s linear speed as base for conversion)
    omega_rad = omega / 1.0

    # Check if angular velocity is zero (straight line movement)
    if abs(omega_rad) < 1e-6:
        return float('inf')

    # Calculate turn radius
    # Absolute value as turn radius is always positive
    turn_radius = abs(v / omega_rad)

    return turn_radius

# Example usage
original_v = 0.0     # 1 m/s
original_omega = -10  # 30 degrees/s (left turn)
min_radius = 1.0     # 2 meters

adjusted_v = enforce_min_turn_radius(original_v, original_omega, min_radius)
turn_radius = calculate_turn_radius(adjusted_v, original_omega)

# Print the adjusted velocities
print("Adjusted Linear Velocity:", adjusted_v, "m/s")
print("Adjusted Angular Velocity:", original_omega, "degrees/s")
print("Turn Radius:", turn_radius, "m")

# Example usage
linear_velocity_percent = 0.2  # 50% of top speed
angular_velocity_percent = -0.2  # 30% of top speed (scaled)
top_speed = 2.0                # 1 m/s

adjusted_linear_percent, adjusted_angular_percent = enforce_min_turn_radius_fraction(
    linear_velocity_percent, angular_velocity_percent, min_radius, top_speed)

print("Adjusted Linear Velocity Percent:", adjusted_linear_percent)
print("Adjusted Angular Velocity Percent:", adjusted_angular_percent)

left_motor, right_motor = differential_drive(adjusted_linear_percent, adjusted_angular_percent)
print("Left Motor power:", left_motor)
print("Right Motor power:", right_motor)

turn_radius = calculate_turn_radius_percent(adjusted_linear_percent, adjusted_angular_percent, top_speed)
print("Turn Radius:", turn_radius, "meters")
