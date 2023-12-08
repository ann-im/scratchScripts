import math

class Vector3D:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def dot(self, other):
        return self.x * other.x + self.y * other.y + self.z * other.z

    def __sub__(self, other):
        return Vector3D(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar):
        return Vector3D(self.x * scalar, self.y * scalar, self.z * scalar)

    def getX(self):
        return self.x

    def getY(self):
        return self.y

def computeYaw(mag_x, mag_y, mag_z, accel_x, accel_y, accel_z):
    vector_mag = Vector3D(mag_x, mag_y, mag_z)
    vector_down = Vector3D(accel_x, accel_y, accel_z)
    scale_factor = vector_mag.dot(vector_down) / vector_down.dot(vector_down)
    vector_north = vector_mag - vector_down * scale_factor
    return math.atan2(vector_north.getX(), vector_north.getY()) * 180 / math.pi

# Example usage
x_mag = -1.17
y_mag = 17.251
z_mag = -27.456
x_accel = 0.003
y_accel = -0.009
z_accel = 0.999
yaw = computeYaw(x_mag, y_mag, z_mag, x_accel, y_accel, z_accel)
print("WIT", yaw)
yaw = computeYaw(y_mag, x_mag, z_mag, x_accel, y_accel, z_accel)
print("Viam", yaw)
