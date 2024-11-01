import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import griddata
from mpl_toolkits.mplot3d import Axes3D
from PIL import Image
from scipy.ndimage import zoom

# Load the CSV file and parse coordinates
file_path = "coordinates.csv"
data = pd.read_csv(file_path)
data[['latitude', 'longitude']] = data['coordinates'].str.extract(r'(-?\d+\.\d+),\s*(-?\d+\.\d+)').astype(float)

# Define the map corners



top_left = (144.900807, -37.673467)
top_right = (145.244541, -37.675475)
bottom_left = (144.913295, -37.933589)
bottom_right = (145.237128, -37.931713)

# Convert coordinates to kilometers
def to_km(lon, lat):
    return lon * 111.32, lat * 110.574

# Convert data points to km
x = data['longitude'] * 111.32
y = data['latitude'] * 110.574
z = data['TT min']

# Convert map corners to km
x_min, y_max = to_km(*top_left)
x_max, _ = to_km(*top_right)
_, y_min = to_km(*bottom_left)

# Create a grid for interpolation (100x100 points)
grid_x, grid_y = np.mgrid[x_min:x_max:100j, y_min:y_max:100j]
grid_z = griddata((x, y), z, (grid_x, grid_y), method='cubic')

# Load and prepare the map image
map_img = Image.open("map.png").convert("RGB")
# Resize map to match grid dimensions
map_img = map_img.resize((100, 100))
map_img_array = np.array(map_img) / 255.0  # Normalize to [0,1]

# Create the 3D plot
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Plot the map as the base layer
z_map = np.full_like(grid_x, np.nanmin(grid_z) - 1)  # Offset map slightly below data
ax.plot_surface(grid_x, grid_y, z_map, rstride=1, cstride=1, 
                facecolors=map_img_array, shade=False)

# Plot the interpolated surface
surf = ax.plot_surface(grid_x, grid_y, grid_z, cmap='viridis', 
                      edgecolor='none', alpha=0.7)

# Add scatter points for the actual data
ax.scatter(x, y, z, color='r', marker='o', s=20, label="Data Points")

# Customize the plot
ax.set_xlabel("Longitude (km)")
ax.set_ylabel("Latitude (km)")
ax.set_zlabel("Distance (TT min)")
fig.colorbar(surf, ax=ax, shrink=0.5, aspect=5, 
             label="Interpolated Distance (TT min)")
plt.legend()

plt.show()
