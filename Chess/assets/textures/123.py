from PIL import Image, ImageDraw, ImageFilter
import math

def create_radial_gradient_circle():
    # Создаем прозрачное изображение 400x400
    image = Image.new('RGBA', (400, 400), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    
    # Координаты центра и радиус круга
    center_x, center_y = 200, 200
    radius = 200  # Немного меньше половины изображения
    
    # Цвет в центре (красный без прозрачности)
    center_color = (255, 0, 0, 255)
    
    # Создаем радиальный градиент
    for x in range(400):
        for y in range(400):
            # Рассчитываем расстояние от центра
            distance = math.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
            
            # Если точка внутри круга
            if distance <= radius:
                # Нормализуем расстояние (от 0 в центре до 1 на краю)
                normalized_distance = distance / radius
                
                # Рассчитываем альфа-канал (полная непрозрачность в центре, полная прозрачность на краю)
                alpha = int(255 * (1 - normalized_distance))
                
                # Устанавливаем цвет с учетом градиента прозрачности
                color = (255, 0, 0, alpha)
                image.putpixel((x, y), color)
    
    return image

# Создаем изображение
result_image = create_radial_gradient_circle()

# Сохраняем изображение
result_image.save('red_gradient_circle.png')
print("Изображение сохранено как 'red_gradient_circle.png'")

# Показываем изображение (если нужно)
# result_image.show()
