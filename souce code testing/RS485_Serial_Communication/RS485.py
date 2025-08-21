import serial

ser = serial.Serial('COM7', 9600, timeout=1)  # Ganti COM7 sesuai PC
print("Mendengarkan RS485...")

while True:
    data = ser.readline().decode(errors='ignore').strip()
    if data:
        print("Diterima:", data)

