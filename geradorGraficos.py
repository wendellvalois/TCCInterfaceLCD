from xdrlib import ConversionError
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import tkinter as tk
from tkinter import ttk, scrolledtext, Frame, messagebox
from tkinter import Menu


def conta_sensores(dicionario: dict):
    lista_sensores = list()
    for index, item in dicionario.items():
        if f"{item['sensor']}{item['numero']}" not in lista_sensores:
            lista_sensores.append(f"{item['sensor']}{item['numero']}")
    print(lista_sensores)
    print(len(lista_sensores))
    return len(lista_sensores)


def converte_para_nested_dictionary(captura_texto_bruto):
    """ Cria lista de dicionarios """
    lista_capturas = list()
    for item in captura_texto_bruto.split("\n"):
        captura = item.split(" ")
        if len(captura) == 3:
            lista_capturas.append({'sensor': captura[0], 'numero': int(captura[1]), 'valor': float(captura[2])})
    # Cria dicionários de dicionarios (Nested Dictionary)
    dicionario_capturas = dict()
    for i, inner_dic in enumerate(lista_capturas):
        dicionario_capturas[i] = inner_dic
    print(lista_capturas)
    print(dicionario_capturas)
    return dicionario_capturas


class FerramentaTCC:
    def __init__(self):
        # Cria instancia
        self.win = tk.Tk()

        # Add a title
        self.win.title("Gerador de gráficos")

        self.frame_header = ttk.Frame(master=self.win)
        self.frame_header.pack(side='left')

        # ========  Criação de Elementos
        # Hora
        self.aLabelHora = ttk.Label(self.frame_header, text="Hora Inicio")
        self.hora = tk.StringVar()
        self.hora_inserida = ttk.Entry(self.frame_header, width=2, textvariable=self.hora)

        # Minuto
        self.minuto = tk.StringVar()
        self.minuto_inserido = ttk.Entry(self.frame_header, width=2, textvariable=self.minuto)

        self.text_area = ttk.Frame(self.win)
        self.text_area.pack(side='left')

        # Zona de texto para entrada, label e botão
        self.scr = scrolledtext.ScrolledText(self.frame_header, width=30, height=30, wrap=tk.WORD)
        self.aLabel = ttk.Label(self.frame_header, text="Gerar grafico")
        self.botao_gerar_grafico = ttk.Button(self.frame_header, text="Gerar Grafico", command=self.click_me)
        # ========
        self.place_interface()

        #####################################################################################

    def place_interface(self):
        self.aLabelHora.grid(column=0, row=0, sticky='E', columnspan=2)
        self.hora_inserida.grid(column=0, row=1, sticky='E')
        # self.aLabelMinuto.grid(column=1, row=0, sticky='W')
        self.minuto_inserido.grid(column=1, row=1, sticky='W')

        self.scr.grid(column=0, row=2, columnspan=4)
        # self.aLabel.grid(column=0, row=0)
        self.botao_gerar_grafico.grid(column=0, row=3)

    def click_me(self):
        captura_texto_bruto = self.scr.get("1.0", tk.END)

        dicionario_capturas = converte_para_nested_dictionary(captura_texto_bruto)

        # for dict_id, value in dicionario_capturas.items():
        #     print(f"CAPTURA {dict_id}")
        #     for key in value:
        #         print(f"{key}: {value[key]}")
        #     print()

        self.plot_grafico(dicionario_capturas)

    def plot_grafico(self, captura: dict):

        try:
            hora = int(self.hora.get())
            minuto = int(self.minuto.get())
        except ValueError:
            messagebox.showerror(title=None, message="Horário incompativel")

        numero_coletas = int(len(captura) / conta_sensores(captura))
        # x_values = range(0, numero_coletas)

        # Imprimir por tempo
        import datetime
        horario_inicio = datetime.datetime.now()
        horario_inicio = horario_inicio.replace(hour=hora, minute=minuto, second=0)

        x_values = [horario_inicio + datetime.timedelta(minutes=i) for i in range(numero_coletas)]

        termometro1 = list()
        termometro2 = list()
        termometro3 = list()
        termometro4 = list()

        for indice, value in captura.items():
            if value['numero'] == 1:
                termometro1.append(value['valor'])
            if value['numero'] == 2:
                termometro2.append(value['valor'])
            if value['numero'] == 3:
                termometro3.append(value['valor'])
            if value['numero'] == 4:
                termometro4.append(value['valor'])
        # --------------------------------------------------------------
        fig = Figure(figsize=(12, 5), facecolor='white')
        # --------------------------------------------------------------
        axis = fig.add_subplot(111)  # 1 row, 1 column
        #
        t0, = axis.plot(x_values, termometro1, color='black')
        t1, = axis.plot(x_values, termometro2, color='red')
        t2, = axis.plot(x_values, termometro3, color='grey')
        t3, = axis.plot(x_values, termometro4, linestyle='--')

        # para latas coloridas
        # t0, = axis.plot(x_values, termometro1, color='blue')
        # t1, = axis.plot(x_values, termometro2, color='red')
        # t2, = axis.plot(x_values, termometro3, color='gold')
        # t3, = axis.plot(x_values, termometro4, linestyle='--')

        import matplotlib.dates as mdates
        myFmt = mdates.DateFormatter('%H:%M')
        axis.xaxis.set_major_formatter(myFmt)

        axis.set_ylabel('Temperatura em °C')
        axis.set_xlabel('Coleta a cada  1 minuto')

        axis.grid()

        fig.legend((t0, t1, t2, t3), ('Preto', 'Branco', 'Vermelho', "Controle"), 'upper right')
        # fig.legend((t0, t1, t3), ('Preto', 'Branco', "Controle"), 'upper right')
        # fig.legend((t0, t1, t2, t3), ('Azul', 'Vermelho', 'Amarelo', "Controle"), 'upper right')
        # fig.legend((t0, t1, t2), ('Azul', 'Vermelho', 'Amarelo'), 'upper right')

        canvas = FigureCanvasTkAgg(fig, master=self.win)

        self.toolbar_frame = Frame(master=self.win)
        self.toolbar_frame.pack()
        # self.toolbar_frame.grid(row=5, column=4, columnspan=3)
        NavigationToolbar2Tk(canvas, self.toolbar_frame).grid(row=5, column=4, columnspan=3)
        # toolbar.grid(column=0)
        canvas.get_tk_widget().pack(side='right')

        self.frame_header = ttk.Frame(master=self.win)
        self.frame_header.pack()

        # ========  Criação de Elementos
        # Hora
        self.aLabelHora = ttk.Label(self.frame_header, text="Hora Inicio")
        self.hora = tk.StringVar()
        self.hora_inserida = ttk.Entry(self.frame_header, width=2, textvariable=self.hora)

    # Exit GUI cleanly
    def _quit(self):
        self.win.quit()
        self.win.destroy()
        exit()


def main():
    ferramenta = FerramentaTCC()
    ferramenta.win.mainloop()


if __name__ == '__main__':
    main()
