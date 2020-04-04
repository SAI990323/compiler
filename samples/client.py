#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
from PyQt5.QtGui import QFont
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
import os

class Client(QWidget):

  def __init__(self):
    super().__init__()

    layout = QVBoxLayout()
    self.setGeometry(300, 300, 300, 200)
    self.setWindowTitle('client')

    self.btn = QPushButton('Done')
    self.label = QLabel()
    self.text = QTextEdit()
    self.text2 = QTextEdit()

    self.btn.clicked.connect(self.execute)
    self.label.setText("请输入代码:");

    layout.addWidget(self.btn)
    layout.addWidget(self.label)
    layout.addWidget(self.text)
    layout.addWidget(self.text2)


    self.setLayout(layout)


  def execute(self):
    execcute_string = self.text.toPlainText()
    with open('assets/code/ui_code.txt', 'w') as f:
      f.write(execcute_string)
    os.system("./sample_lexer assets/code/ui_code.txt output/output_lexer.txt output/output_lexer.csv")
    s = ""
    with open('output/output_lexer.txt', 'r') as f:
      for line in f:
        s = s + line
    self.text2.setText(s)

if __name__ == '__main__':
  app = QApplication(sys.argv)
  client = Client()
  client.show()
  sys.exit(app.exec_())