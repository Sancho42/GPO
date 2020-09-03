from tkinter import *

root = Tk()
root.title("ГПО - Речевые технологии")
root.geometry('600x400+100+100')


def Hello(event):
    print("Hello world")


#btn = Button(root, text="Click me", width=20, height=2, bg="white", fg="black")
#btn.bind("<Button-1>", Hello)  # при нажатии ЛКМ на кнопку вызывается функция Hello
#btn.pack(side='top')  # расположить кнопку на главном окне

label = Label()
label.pack(side='left')

textbox = Text(font='Arial 14', wrap='word')
textbox.pack()

root.mainloop()
