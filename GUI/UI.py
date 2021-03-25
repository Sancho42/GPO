import tkinter as tk
from tkinter.filedialog import askopenfilename


import filter as fltr
import mask
import pitch_frequency

class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_widgets()



    def create_widgets(self):
        # Label
        self.path_to_file_label = tk.Label(text="Путь к файлу")
        self.path_to_file_label.place(x=10, y=50, width=100)

        # Text PATH to file
        self.path_entry = tk.Entry(root, width=40, justify=tk.LEFT)
        self.path_entry.place(x=110, y=50)
        self.path_entry.insert(0, 'File/spectrum1.bin')

        # Button to filter
        self.filter_button = tk.Button(root)
        self.filter_button["text"] = "Filter"
        self.filter_button["command"] = self.fileFilter
        self.filter_button.place(x=10, y=10)

        # Button to mask
        self.mask_online_botton = tk.Button(root)
        self.mask_online_botton["text"] = "Mask"
        self.mask_online_botton["command"] = self.fileMask
        self.mask_online_botton.place(x=50, y=10)

        # Button to int.1-4gram
        self.int1_4gram_botton = tk.Button(root)
        self.int1_4gram_botton["text"] = "Инт.1-4 грам."
        self.int1_4gram_botton["command"] = self.fileMask
        self.int1_4gram_botton.place(x=90, y=10)

        # Button to sumInt
        self.sumInt_botton = tk.Button(root)
        self.sumInt_botton["text"] = "Сумм.инт."
        self.sumInt_botton["command"] = self.fileMask
        self.sumInt_botton.place(x=175, y=10)

        # Button to otnInt2_9
        self.otnInt2_9_botton = tk.Button(root)
        self.otnInt2_9_botton["text"] = "Отн.инт.2-9"
        self.otnInt2_9_botton["command"] = self.fileMask
        self.otnInt2_9_botton.place(x=245, y=10)

        # Button to otnIntMax4ot
        self.otnIntMax4ot_botton = tk.Button(root)
        self.otnIntMax4ot_botton["text"] = "Отн.интMax/ЧОТ"
        self.otnIntMax4ot_botton["command"] = self.fileMask
        self.otnIntMax4ot_botton.place(x=320, y=10)

        # Button to otnIntMax
        self.otnIntMax_botton = tk.Button(root)
        self.otnIntMax_botton["text"] = "Отн.инт.Max"
        self.otnIntMax_botton["command"] = self.fileMask
        self.otnIntMax_botton.place(x=425, y=10)

        # Button to chot
        self.chot_botton = tk.Button(root)
        self.chot_botton["text"] = "ЧОТ"
        self.chot_botton["command"] = self.pitchFrequency
        self.chot_botton.place(x=505, y=10)

        # Button to chastMax
        self.chastMax_botton = tk.Button(root)
        self.chastMax_botton["text"] = "Част.Max"
        self.chastMax_botton["command"] = self.fileMask
        self.chastMax_botton.place(x=540, y=10)



        # Button to open file
        self.open_file_botton = tk.Button(root)
        self.open_file_botton["text"] = "..."
        self.open_file_botton["command"] = self.open_file
        self.open_file_botton.place(x=360, y=45)

        # Label
        self.Server_IP_label = tk.Label(text="Server IP:")
        self.Server_IP_label.place(x=10, y=75, width=100)

        # Text to Server IP
        self.Server_IP_text = tk.Entry(root, width=16, justify=tk.LEFT)
        self.Server_IP_text.place(x=110, y=75)
        self.Server_IP_text.insert(0, '')

        # Label
        self.Server_PORT_label = tk.Label(text="Server port:")
        self.Server_PORT_label.place(x=10, y=100, width=100)

        # Text to Server PORT
        self.Server_PORT_text = tk.Entry(root, width=7, justify=tk.LEFT)
        self.Server_PORT_text.place(x=110, y=100)
        self.Server_PORT_text.insert(0, '')

        # Button to conect
        self.URL_button = tk.Button(root)
        self.URL_button["text"] = "Connect"
        self.URL_button["command"] = self.open_search
        self.URL_button.place(x=110, y=125)

        # Label
        self.var_download_status = tk.StringVar()
        self.var_download_status.set('Status: ')

        self.download_status_label = tk.Label(text="Status", textvariable=self.var_download_status)
        self.download_status_label.place(x=200, y=125)

        # # Button to filter online
        # self.filter_online_button = tk.Button(root)
        # self.filter_online_button["text"] = "Filter"
        # self.filter_online_button["command"] = self.file_filter_buf
        # self.filter_online_button.place(x=110, y=130)
        #
        # # Button to mask online
        # self.mask_online_botton = tk.Button(root)
        # self.mask_online_botton["text"] = "Mask"
        # self.mask_online_botton["command"] = self.file_mask_buf
        # self.mask_online_botton.place(x=150, y=130)



    def fileFilter(self):
        file = fltr.Filter()
        file.filtered(self.path_entry.get())

    def fileMask(self):
        mask_ = mask.Mask()
        mask_.masked(self.path_entry.get())

    def pitchFrequency(self):
        pitch_freq = pitch_frequency.Pitcher()
        pitch_freq.pitched(self.path_entry.get())

    def open_file(self):
        filename = askopenfilename()
        if filename != '':
            self.path_entry.delete(0, 'end')
            self.path_entry.insert(0, filename)

    def download_file(self):
        self.var_download_status.set('Status: OK')
        # TODO

    def file_filter_buf(self, file_path):
        file = fltr.Filter()
        file.filtered(file_path)

    def file_mask_buf(self, file_path):
        mask_ = mask.Mask()
        mask_.masked(file_path)

    def open_search(self):
        self.searchWin = tk.Toplevel()
        self.searchWin.minsize(520, 400)

        # Label
        self.search_label = tk.Label(self.searchWin, text="Поиск:")
        self.search_label.place(x=10, y=10, width=100)

        # Text to search
        self.search_text = tk.Entry(self.searchWin, width=45, justify=tk.LEFT)
        self.search_text.place(x=110, y=10)
        self.search_text.insert(0, '')

        # Button to search
        self.search_button = tk.Button(self.searchWin)
        self.search_button["text"] = "Найти"
        self.search_button["command"] = self.download_file
        self.search_button.place(x=400, y=5)

        # Label
        self.result_label = tk.Label(self.searchWin, text="Результат")
        self.result_label.place(x=10, y=40, width=100)

        # Text of searching
        self.result_text = tk.Text(self.searchWin, width=49, height=20)
        self.result_text.place(x=110, y=40)

        # Checkbutton of search file
        varFail = tk.BooleanVar()
        varFail.set(1)
        self.search_file_cb = tk.Checkbutton(self.searchWin, text="Файл", variable=varFail, onvalue=1, offvalue=0)
        self.search_file_cb.place(x=10, y=80)

        # Checkbutton of search dictor
        varDictor = tk.BooleanVar()
        varDictor.set(1)
        self.search_dictor_cb = tk.Checkbutton(self.searchWin, text="Диктор", variable=varDictor, onvalue=1, offvalue=0)
        self.search_dictor_cb.place(x=10, y=110)

        # Button to download
        self.download_result_button = tk.Button(self.searchWin)
        self.download_result_button["text"] = "Скачать"
        self.download_result_button["command"] = self.download_file
        self.download_result_button.place(x=450, y=5)





root = tk.Tk()
app = Application(master=root)

root.title("ГПО - Речевые технологии")
root.geometry('600x400+100+100')

app.mainloop()
