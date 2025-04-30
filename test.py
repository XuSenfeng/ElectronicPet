'''
Descripttion: 
Author: Xvsenfeng helloworldjiao@163.com
LastEditors: Xvsenfeng helloworldjiao@163.com
Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
'''
import tkinter as tk

# 创建主窗口
root = tk.Tk()
root.title("示例窗口")

# 创建标签
label = tk.Label(root, text="Hello, Tkinter!")
label.pack()

# 创建按钮
button = tk.Button(root, text="点击我", command=lambda: print("按钮被点击"))
button.pack()

# 进入主循环
root.mainloop()