import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
import time

# === CONFIGURAÇÕES ===
PORTA_SERIAL = 'COM7'
BAUD_RATE = 9600
TEMPO_TOTAL_SEGUNDOS = 300  # 5 minutos
INTERVALO_MS = 100

# === INICIALIZAÇÃO SERIAL ===
ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
time.sleep(2)
ser.flushInput()

# === DADOS ===
tempo = deque()
distancia = deque()

# === CONFIGURAÇÃO DO PLOT ===
fig, ax = plt.subplots()
linha, = ax.plot([], [], label="Distância (cm)", color='blue')

ax.set_ylim(0, 50)
ax.set_xlim(0, TEMPO_TOTAL_SEGUNDOS)
ax.set_title("Gráfico em tempo real - Sensor Ultrassônico")
ax.set_xlabel("Tempo (s)")
ax.set_ylabel("Distância (cm)")
ax.legend(loc="upper right")
ax.grid(True)

# === INÍCIO DO TEMPO REAL ===
tempo_inicio = time.time()

# === FUNÇÃO DE ATUALIZAÇÃO ===
def update(frame):
    tempo_atual = time.time() - tempo_inicio
    if tempo_atual >= TEMPO_TOTAL_SEGUNDOS:
        plt.close(fig)  # Fecha o gráfico após 5 minutos reais
        return linha,

    if ser.in_waiting:
        try:
            linha_serial = ser.readline().decode('utf-8').strip()
            p = float(linha_serial)
            tempo.append(tempo_atual)
            distancia.append(p)
            linha.set_data(tempo, distancia)
            ax.set_xlim(max(0, tempo_atual - TEMPO_TOTAL_SEGUNDOS), tempo_atual + 1)
        except (ValueError, UnicodeDecodeError):
            pass

    return linha,

# === ANIMAÇÃO ===
ani = animation.FuncAnimation(fig, update, interval=INTERVALO_MS)
plt.tight_layout()
plt.show()

# === FECHAR SERIAL ===
ser.close()
