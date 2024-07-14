import tkinter as tk
import tkinter.messagebox as tkm

def buttonClick(event):
    btn = event.widget
    txt = btn["text"]
    #入力欄と押されたボタンのテキストを渡す
    is_double_symbol = preventDoubleSymbol(entry.get(),txt)
    #記号が二重になっていなければ挿入
    if not is_double_symbol or len(entry.get()) == 0:
        entry.insert(tk.END, txt)

def calcPow():
    pass

def preventDoubleSymbol(txt,btn_txt):
    #入力欄に1文字もない場合、二重記号扱いで終了
    if len(txt) == 0:
        return False
    last = txt[-1] #文字列の最後尾
    #文字列の最後尾が記号かつボタンのテキストが記号の場合は二重記号とする
    if last in "+|-|×|÷" and btn_txt in "+|-|×|÷":
        return True
    else:
        return False

def clickEqual(event):
    eqn = entry.get()
    #最後尾が数字じゃない場合は計算を実行しない
    if not eqn[-1].isnumeric():
        return
    eqn = eqn.replace("×", "*")
    eqn = eqn.replace("÷", "/")
    if "/0" in eqn:
        res = "0除算エラー"
    else:
        res = eval(eqn)
    entry.delete(0, tk.END)
    entry.insert(tk.END, res)

root = tk.Tk()
root.title("電卓")
root.geometry("400x600")

button_num = [str(i) for i in range(9, -1, -1)]
button_txt = ["+", "-", "×", "÷"]
button_num.extend(button_txt)

j=1
k=0
fontsize = 30
for i in button_num:
    button = tk.Button(root, width=4, height=1, text=f"{i}",font=("",fontsize))
    button.bind("<1>",buttonClick)
    button.grid(row=j,column=k)
    k+=1
    if k==3:
        k=0
        j+=1

button = tk.Button(root, width=4, height=1, text="x²",font=("",fontsize))
button.bind("<1>",calcPow)
button.grid(row=j,column=k)

j=j+1
k=0
button = tk.Button(root, width=4, height=1, text="=",font=("",fontsize))
button.bind("<1>",clickEqual)
button.grid(row=j,column=k)

entry = tk.Entry(justify="right",width=11, font=("",fontsize+10))
entry.grid(row=0,column=0,columnspan=4)

#entry.insert(tk.END, "fugapiyo")
#entry.pack()

'''
label = tk.Label(root,
                text="ラベル",
                font=("メイリオUI",20)
                )
label.grid(row=0,column=0)
'''

root.mainloop()