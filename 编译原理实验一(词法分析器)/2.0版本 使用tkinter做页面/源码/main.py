import tkinter as tk
from tkinter.filedialog import askopenfilename
import tkinter.messagebox
import os
import sys

def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_path, relative_path)

class MyView:
    def __init__(self):
        self.root = tk.Tk()
        self.root.geometry('500x600')
        self.root.title('编译原理实验一')
        self.root.resizable(False, False)

        self.btn1 = tk.Button(self.root, text='选择文件', command=self.btn1_callback,
                              height=2, width=16, font=16, bg="lightblue")
        self.btn2 = tk.Button(self.root, text='开始词法分析', command=self.btn2_callback,
                              height=2, width=16, font=16, bg="lightblue")
        self.btn1.place(x=60, y=30)
        self.btn2.place(x=300, y=30)
        self.label = tk.Label(self.root, text='输出框', font=13)
        self.label.place(x=220, y=80)
        self.list = tk.Listbox(self.root, width=65, height=26)
        self.list.place(x=20, y=110)

        self.filepath = None

    def btn1_callback(self):
        self.filepath = askopenfilename()
        print(self.filepath)

    def btn2_callback(self):
        print("开始转换")
        if self.filepath is None or self.filepath == '':
            tk.messagebox.showerror('ERROR', '请先选择输入文件')
        elif not os.path.exists(resource_path('cLexer.exe')):
            l = os.listdir()
            s = ','.join(l)
            tk.messagebox.showerror('ERROR', f'当前目录下不存在cLexer.exe程序!\n当前目录下的文件有{s}.')
        else:
            self.list.delete(0, tk.END)
            cmd = f'{resource_path("cLexer.exe")} "{self.filepath}"'
            print(cmd)
            a = os.popen(cmd)
            result_list = a.buffer.readlines()
            for item in result_list:
                item = item.decode('utf8')
                self.list.insert(tk.END, item)

    def run(self):
        self.root.mainloop()


if __name__ == '__main__':
    view = MyView()
    view.run()
