from PIL import Image
import numpy as np

# Definir los 7 colores de la pantalla E Ink
eink_colors = np.array([
    [255, 255, 255],  # Blanco
    [0, 0, 0],        # Negro
    [255, 0, 0],      # Rojo
    [0, 255, 0],      # Verde
    [0, 0, 255],      # Azul
    [255, 255, 0],    # Amarillo
    [255, 165, 0],    # Naranja
])

# Función para calcular la distancia cuadrada entre dos colores
def squared_distance(c1, c2):
    return np.sum((c1 - c2) ** 2)

# Función para obtener el color más cercano de la paleta
def closest_color(pixel, eink_colors):
    distances = np.array([squared_distance(pixel, color) for color in eink_colors])
    return np.argmin(distances)  # Regresa el índice del color más cercano

# Función para convertir la imagen a los 7 colores y guardarla en un array
def convert_to_eink(image_path, eink_colors, width, height):
    # Abrir la imagen y convertirla a formato RGB
    img = Image.open(image_path)
    img = img.convert('RGB')  # Asegurarse de que la imagen esté en formato RGB

    # Redimensionar la imagen a las dimensiones especificadas
    img = img.resize((width, height))

    # Crear una imagen de paleta
    palette_image = Image.new("P", (1, 1))

    # Crear la paleta de los 7 colores
    palette = []
    for color in eink_colors:
        palette.extend(color)
    palette += [0] * (256 - len(palette))  # Completar la paleta hasta 256 colores

    # Aplicar la paleta
    palette_image.putpalette(palette)

    # Convertir la imagen al modo "P" con la paleta personalizada
    img = img.convert("P", palette=palette_image.getpalette())

    # Convertir la imagen a un array de numpy
    img_data = np.array(img)

    # Crear un array para almacenar los valores de 3 bits por color
    bit_array = []

    # Recorrer la imagen y agrupar los colores en un byte de 8 bits
    for row in img_data:
        for i in range(0, len(row), 3):  # Agrupar cada 3 píxeles
            # Los valores de los 3 píxeles se almacenarán en los 3 bits correspondientes
            byte_value = 0
            for j in range(3):
                if i + j < len(row):
                    color_index = row[i + j]  # Índice del color en la paleta
                    byte_value |= color_index << (3 * (2 - j))  # Colocar cada índice en su lugar correspondiente
            bit_array.append(byte_value)

    # Convertir el array de bits en un array de u8
    u8_array = np.array(bit_array, dtype=np.uint8)

    return u8_array

# Uso
u8_array = convert_to_eink("image.jpg", eink_colors, 250, 250)

# Mostrar el resultado en un array de uint8
print(u8_array)

# Opcional: Guardar el array en un archivo binario
u8_array.tofile("image_u8_array.bin")
