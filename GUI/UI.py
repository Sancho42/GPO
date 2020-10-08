import tkinter as tk


import sancho.filter as fltr
import sancho.mask as mask

class Application(tk.Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_widgets()

    def create_widgets(self):
        # Label
        self.l1 = tk.Label(text="Path to file")
        self.l1.place(x=10, y=10, width=100)

        # Text PATH to file
        self.pathEntry = tk.Entry(root, width=40, justify=tk.LEFT)
        self.pathEntry.place(x=110, y=10)
        self.pathEntry.insert(0, '../File/spectrum1.bin')

        # Button to filter
        self.filter_button = tk.Button(root)
        self.filter_button["text"] = "Filter"
        self.filter_button["command"] = self.fileFilter
        self.filter_button.place(x=110, y=35)

        # Button to mask
        self.mask_botton = tk.Button(root)
        self.mask_botton["text"] = "Mask"
        self.mask_botton["command"] = self.fileMask
        self.mask_botton.place(x=150, y=35)

        # Label
        self.l2 = tk.Label(text="URL to file")
        self.l2.place(x=10, y=75, width=100)

        # Text URL to speach
        self.pathURL = tk.Entry(root, width=40, justify=tk.LEFT)
        self.pathURL.place(x=110, y=75)
        self.pathURL.insert(0, '')

        # Button to load file by URL
        self.URL_button = tk.Button(root)
        self.URL_button["text"] = "Download"
        self.URL_button["command"] = self.fileFilter
        self.URL_button.place(x=110, y=100)


    def fileFilter(self):
        file = fltr.Filter()
        file.filtered(self.pathEntry.get())

    def fileMask(self):
        mask_ = mask.Mask()
        mask_.masked(self.pathEntry.get())


root = tk.Tk()
app = Application(master=root)

root.title("ГПО - Речевые технологии")
root.geometry('600x400+100+100')

app.mainloop()
