import sys
import csv
import chardet
from PySide6.QtWidgets import (
    QApplication, QMainWindow, QTableView, QWidget,
    QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton,
    QMessageBox, QDialog, QTextEdit, QScrollArea  # 新增导入
)
from PySide6.QtGui import (
    QStandardItemModel, QStandardItem,
    QFont, QFontDatabase
)
from PySide6.QtCore import Qt

class CSVManager:
    def __init__(self, filename):
        self.filename = filename
        self.headers = []
        self.data = []
        self.load_data()

    def load_data(self):
        try:
            # 检测文件编码
            with open(self.filename, 'rb') as f:
                raw_data = f.read()
                result = chardet.detect(raw_data)
                encoding = result['encoding'] or 'utf-8-sig'

            # 使用正确编码读取
            with open(self.filename, 'r', encoding=encoding) as f:
                reader = csv.reader(f)
                self.headers = next(reader)
                self.data = [row for row in reader]
        except (FileNotFoundError, StopIteration):
            self.create_empty_file()

    def save_data(self):
        with open(self.filename, 'w', encoding='utf-8-sig', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(self.headers)
            writer.writerows(self.data)

    def create_empty_file(self):
        default_headers = [
            "tm_sec", "tm_min", "tm_hour", "re_mday", "re_mon",
            "re_year", "re_wday", "random_l", "random_h", "message", "function_id"
        ]
        self.headers = default_headers
        self.data = []
        self.save_data()

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("电子宠物日程管理系统")
        self.resize(1000, 600)
        
        # 初始化字体
        self.setup_fonts()
        
        # 初始化组件
        self.manager = CSVManager("electronic_pet.csv")
        self.init_ui()
        self.load_data_to_table()

    def setup_fonts(self):
        """设置跨平台中文字体"""
        font_families = [
            "Microsoft YaHei",    # Windows
            "SimSun",             # Windows
            "WenQuanYi Micro Hei",# Linux
            "Arial Unicode MS",   # macOS
            "Noto Sans CJK SC"   # 通用字体
        ]
        
        # 创建字体对象
        font = QFont()
        font.setPointSize(10)
        
        # 检测可用字体
        available_fonts = QFontDatabase.families()
        for family in font_families:
            if family in available_fonts:
                font.setFamily(family)
                break
        else:
            QMessageBox.warning(
                self, "font error",
                "未找到中文字体，建议安装微软雅黑或思源黑体"
            )
        
        # 设置全局字体
        QApplication.instance().setFont(font)

    def init_ui(self):
        # 主容器
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # 表格视图（设置额外字体保证）
        self.table_view = QTableView()
        self.table_view.setFont(QApplication.font())
        self.model = QStandardItemModel()
        self.table_view.setModel(self.model)
        main_layout.addWidget(self.table_view)

        # 输入区域
        input_layout = QHBoxLayout()
        self.input_fields = []

        input_labels = [
            "秒 (tm_sec)", "分 (tm_min)", "时 (tm_hour)",
            "日 (re_mday)", "月 (re_mon)", "年 (re_year)",
            "星期 (re_wday)", "随机下限 (random_l)", "随机上限 (random_h)","函数标号 (function_id)",
            "消息 (message)"
        ]

        for label in input_labels:
            input_field = QLineEdit()
            input_field.setPlaceholderText(label)  # 设置提示信息
            input_field.setFixedWidth(100)        # 稍微加宽输入框
            input_field.setFont(QApplication.font())
            self.input_fields.append(input_field)
            input_layout.addWidget(input_field)

        # 按钮区域
        # 按钮区域修改
        btn_layout = QHBoxLayout()
        
        # 添加记录按钮
        self.add_btn = QPushButton("添加记录")
        self.add_btn.clicked.connect(self.add_record)
        
        # 删除记录按钮
        self.del_btn = QPushButton("删除记录")
        self.del_btn.clicked.connect(self.delete_record)
        
        # 新增手册按钮
        self.manual_btn = QPushButton("使用手册")
        self.manual_btn.clicked.connect(self.show_manual)
        self.manual_btn.setToolTip("查看程序使用说明")  # 添加悬停提示

        # 按钮布局
        btn_layout.addWidget(self.add_btn)
        btn_layout.addWidget(self.del_btn)
        btn_layout.addWidget(self.manual_btn)
        btn_layout.addStretch(1)  # 添加弹性空间保持右对齐

        main_layout.addLayout(input_layout)
        main_layout.addLayout(btn_layout)

    def show_manual(self):
        """显示使用手册"""
        manual_text = """电子宠物日程管理系统使用手册

一、基本操作
1. 添加记录：填写所有输入框后点击「添加记录」
   - 时间字段范围：
     • 秒/分：0-59
     • 时：0-23
     • 日：1-31
     • 月：1-12
     • 年：0-9999
   - 星期字段：0-7
    >（负数表示周期, 比如在天的时候使用-2, 表示以2天为单位进行循环, 
    > 星期几的选择默认使用一周为循环单位, 不同周期的选择建议使用
    > random_l和random_h进行设置, 统一使用-1作为循环标志）

2. 删除记录：选中表格行后点击「删除记录」

二、特殊功能
1. 随机事件配置：
   - random_l=0时：按负数位置周期触发
   - random_l≠0时：在负数位置基础上随机触发

2. 消息格式：
   - 支持中文和特殊符号
   - 可使用时间变量：{hour}时{min}分

三、数据管理
1. 文件自动保存
2. CSV文件编码：UTF-8 with BOM
3. 注释行以#开头

四、注意事项
• 请勿删除表头行
• 修改日期字段后需要重新启动生效
• 建议定期备份数据文件"""

        # 创建带滚动条的对话框
        dialog = QDialog(self)
        dialog.setWindowTitle("系统使用手册")
        dialog.resize(600, 400)
        
        layout = QVBoxLayout(dialog)
        
        # 添加文本编辑框
        text_edit = QTextEdit()
        text_edit.setReadOnly(True)
        text_edit.setFont(QFont("Microsoft YaHei", 10))
        text_edit.setText(manual_text)
        
        # 添加滚动条
        scroll = QScrollArea()
        scroll.setWidget(text_edit)
        scroll.setWidgetResizable(True)
        
        layout.addWidget(scroll)
        
        # 添加关闭按钮
        close_btn = QPushButton("关闭")
        close_btn.clicked.connect(dialog.close)
        layout.addWidget(close_btn)

        dialog.exec()

    def load_data_to_table(self):
        # 设置表头（确保中文显示）
        self.model.setHorizontalHeaderLabels(self.manager.headers)
        self.table_view.setColumnWidth(10, 300)  # 防止数据加载覆盖设置
        for col in range(10):  # 前9列
            self.table_view.setColumnWidth(col, 80)
        # 加载数据
        self.model.setRowCount(len(self.manager.data))
        cycle_message = ["(循环秒)","(循环分)", "(循环时)", "(循环天)", "(循环月)", "(循环年)", "(循环周)"]
        for row_idx, row_data in enumerate(self.manager.data):
            cycle_flag = False
            for col_idx, value in enumerate(row_data):
                # 处理数据类型
                if col_idx != 10:  # 最后一列为消息
                    if not cycle_flag or col_idx >= 7:
                        if int(value) < 0:
                            cycle_flag = True 
                            # 设置字体为红色
                            value = str(0 - int(value))
                            if col_idx == 6:
                                # 将星期转换为中文
                                week_days = ["0","日", "一", "二", "三", "四", "五", "六"]
                                value = week_days[int(value)]
                            value += cycle_message[col_idx]
                            item = QStandardItem(value)
                            item.setForeground(Qt.red)
                            
                        else:
                            if col_idx == 6:
                                # 将星期转换为中文
                                week_days = ["0","日", "一", "二", "三", "四", "五", "六"]
                                value = week_days[int(value)]
                            item = QStandardItem(value)
                    else:
                        item = QStandardItem("")
                else:
                    item = QStandardItem(value)
                    


                # 设置单元格内容
                item.setTextAlignment(Qt.AlignCenter)
                self.model.setItem(row_idx, col_idx, item)

    def add_record(self):
        new_data = [field.text().strip() or '0' for field in self.input_fields]
        
        # if not self.manager.validate_entry(new_data):
        #     return
        
        # 创建居中对齐的数据项
        items = []
        for value in new_data:
            item = QStandardItem(value)
            item.setTextAlignment(Qt.AlignCenter)  # 设置居中对齐
            items.append(item)
        
        self.model.appendRow(items)
        self.manager.data.append(new_data)
        self.manager.save_data()
        
        # 清空输入框
        for field in self.input_fields:
            field.clear()

    def delete_record(self):
        selected = self.table_view.currentIndex()
        if not selected.isValid():
            QMessageBox.warning(self, "选择错误", "请先选择要删除的记录", QMessageBox.Ok)
            return
        
        reply = QMessageBox.question(
            self, "确认删除", 
            "确定要删除选中的记录吗？",
            QMessageBox.Yes | QMessageBox.No
        )
        
        if reply == QMessageBox.Yes:
            row = selected.row()
            self.model.removeRow(row)
            del self.manager.data[row]
            self.manager.save_data()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    # 强制设置备选字体（如果前面检测失败）
    font = QFont()
    font.setFamilies(["Microsoft YaHei", "SimSun"])
    font.setPointSize(10)
    app.setFont(font)
    
    window = MainWindow()
    window.show()
    sys.exit(app.exec())