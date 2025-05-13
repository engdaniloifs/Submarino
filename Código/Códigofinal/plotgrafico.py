import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import time

# === CONFIGURAÇÕES ===
PORTA_SERIAL = 'COM5'
BAUD_RATE = 9600
DURACAO_SEGUNDOS = 300  # 5 minutos
INTERVALO_MS = 100

# === INICIALIZAÇÃO SERIAL ===
ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
time.sleep(2)
ser.flushInput()

# === DADOS PARA PLOTAGEM ===
tempo = deque()
profundidade = deque()
setpoint = deque()

# === CONFIGURAÇÃO DO PLOT ===
fig, ax = plt.subplots()
linha1, = ax.plot([], [], label="Profundidade (cm)", color='blue')
linha2, = ax.plot([], [], label="Setpoint (cm)", color='red')

ax.set_ylim(0, 20)
ax.set_xlim(0, DURACAO_SEGUNDOS)
ax.set_title("Gráfico em tempo real - Arduino (5 minutos)")
ax.set_xlabel("Tempo (s)")
ax.set_ylabel("Profundidade (cm)")
ax.legend(loc="upper right")
ax.grid(True)

# === INÍCIO DO TEMPO REAL ===
inicio = time.time()

# === FUNÇÃO DE ATUALIZAÇÃO ===
def update(frame):
    tempo_atual = time.time() - inicio
    if tempo_atual >= DURACAO_SEGUNDOS:
        plt.close(fig)
        return linha1, linha2

    if ser.in_waiting:
        try:
            linha_serial = ser.readline().decode('utf-8').strip()
            valores = linha_serial.split('\t')
            if len(valores) == 2:
                p = float(valores[0])
                s = float(valores[1])
                tempo.append(tempo_atual)
                profundidade.append(p)
                setpoint.append(s)
                linha1.set_data(tempo, profundidade)
                linha2.set_data(tempo, setpoint)
        except (ValueError, UnicodeDecodeError):
            pass

    return linha1, linha2

# === ANIMAÇÃO ===
ani = animation.FuncAnimation(fig, update, interval=INTERVALO_MS)
plt.tight_layout()
plt.show()

# === FECHAR SERIAL ===
ser.close()
