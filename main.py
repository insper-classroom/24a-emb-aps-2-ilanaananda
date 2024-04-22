import serial
import uinput

ser = serial.Serial('/dev/ttyACM0', 9600) # Mude a porta para rfcomm0 se estiver usando bluetooth no linux
# Caso você esteja usando windows você deveria definir uma porta fixa para seu dispositivo (para facilitar sua vida mesmo)
# Siga esse tutorial https://community.element14.com/technologies/internet-of-things/b/blog/posts/standard-serial-over-bluetooth-on-windows-10 e mude o código acima para algo como: ser = serial.Serial('COMX', 9600) (onde X é o número desejado)

# (Mais códigos aqui https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h?h=v4.7)
buttons = [uinput.BTN_A, # 0 x
    uinput.BTN_B, # 1 o
    uinput.BTN_X, # 2 quadrado
    uinput.BTN_Y, # 3 triangulo
    uinput.KEY_F4, # 4 f4
    uinput.KEY_ESC # 5 esc
          ]
axes = [uinput.ABS_X, uinput.ABS_Y]
button_quantity = 6 # Quantidade de botões no controlador (sem contar com o joystick)

# Criando gamepad emulado
device = uinput.Device(buttons + axes)


# Função para analisar os dados recebidos do dispositivo externo
def parse_data(data):
    """
    Esta função analisa os dados recebidos do dispositivo externo e retorna o botão e o valor correspondentes.

    Argumentos:
    data (bytes): Os dados recebidos do dispositivo externo.

    Retorna:
    int, int: O número do botão e o valor do botão.
    """
    button = data[0]  # Axis no C, o botão apertado
    value = int.from_bytes(data[1:3], byteorder='little', signed=True) # Valor do botão (Apertado, não apertado ou algum outro estado)
    print(f"Received data: {data}")
    print(f"button: {button}, value: {value}")
    return button, value

def emulate_controller(button, value):
    """
    Esta função emula a entrada do controlador no sistema com base no botão e valor recebidos.

    Argumentos:
    button (int): O número do botão a ser emulado.
    value (int): O valor do botão.

    Retorna:
    None
    """
    if button < button_quantity:  # Se o botão estiver entre os botões declarados
        device.emit(buttons[button], value)
    else:  # Se não, ele é um eixo
        device.emit(axes[button - button_quantity], value)

try:
    # Pacote de sync
    while True:
        print('Waiting for sync package...')
        while True:
            data = ser.read(1)
            if data == b'\xff':
                break

        # Lendo 4 bytes da uart
        data = ser.read(3)
        button, value = parse_data(data)
        emulate_controller(button, value)

except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()