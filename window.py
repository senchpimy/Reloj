import sys
from PyQt5 import QtCore, QtGui, QtWidgets, uic
from PyQt5.QtCore import Qt
dias=[]
for i in range(31):
    dias.append(f"Dias {i}")
eth=[1682.500000, 1675.500000, 1664.199951, 1628.500000, 1598.500000, 1588.599976, 1581.199951, 1591.900024, 1593.099976, 1593.099976, 1601.699951, 1631.900024, 1642.300049, 1639.400024, 1628.000000, 1638.099976, 1627.400024, 1624.699951, 1598.500000, 1587.599976, 1585.400024, 1623.199951, 1634.199951, 1637.300049, 1634.900024, 1630.099976, 1628.199951, 1632.699951, 1635.199951, 1633.699951]
xmr=[147.320007, 146.330002, 145.630005, 146.500000, 145.259995, 144.289993, 144.250000, 143.070007, 143.820007, 145.740005, 146.970001, 147.350006, 147.289993, 146.479996, 144.699997, 145.740005, 147.610001, 144.850006, 143.110001, 141.229996, 141.149994, 142.820007, 143.279999, 143.080002, 142.320007, 139.380005, 139.919998, 142.029999, 140.399994, 140.520004]
doge=[0.062352, 0.062069, 0.061790, 0.060970, 0.060636, 0.060714, 0.060868, 0.061460, 0.061598, 0.061542, 0.062031, 0.062409, 0.062514, 0.061968, 0.061981, 0.062320, 0.062033, 0.061655, 0.061105, 0.061243, 0.060733, 0.061960, 0.063527, 0.063335, 0.063335, 0.063757, 0.063732, 0.063178, 0.063383, 0.063549]


number_of_lines_h=8
number_of_lines_w=11
height=int(600/number_of_lines_h)
width=int(448/(number_of_lines_w+1))

def round_to(number):
    return round(number / 5) * 5

def regla_de_tres(val1, proporcion, val2): return int((proporcion*val2)/val1)

class GraphCoordGen:
    def __init__(self, val_max, val_min, coord_max, coord_min):
        dif_uni=val_max-val_min
        dif_coord=coord_max-coord_min
        self.coord_min= coord_min
        self.paso = dif_coord/dif_uni
        self.val_min=val_min
    def form_pen(self,  y):
     dis_y = y - self.val_min
     pasos_tot = dis_y * self.paso
     return int(self.coord_min + pasos_tot)


def draw_grid(painter,pen):
    #Vertical
    number_of_lines_w=10
    desface=10
    for i in range(number_of_lines_w+1):
        print("Hola")
        #painter.drawLine(((i+1)*width)+desface,75,((i+1)*width)+desface,580)
    #Horizontal
    for i in range(number_of_lines_h):
        painter.drawLine(35,(i+1)*height,416,(i+1)*height)
    painter.drawLine(35,580,416,580) #Limite Inferior

def draw_name(data,painter,pen):
    font = QtGui.QFont()
    font.setFamily('Times')
    font.setBold(True)
    font.setPointSize(40)
    painter.setFont(font)
    painter.drawText(20,50,f"HOLA  ${round(data[-1],3)}")
    font.setFamily('Times')
    font.setBold(False)
    font.setPointSize(10)
    painter.setFont(font)

def draw_graph_grid(data,painter,pen):
    res=False
    if max(data)<1.0:
        data = [x*10000 for x in data]
        res=True
    d_min = min(data)
    d_max = max(data)
    tol_val = 20 if d_max>1000 else 2
    t_min = round_to(d_min-tol_val)
    t_max = round_to(d_max+tol_val)
    step = (t_max-t_min)/(number_of_lines_h-1) if not res else round((t_max-t_min)/(number_of_lines_h-1),3)
    for i in range(number_of_lines_h-2): #place numbers
        if not res:
            painter.drawText(15,height*(i+2),f"{t_max-(step*(i+1))}")
        else:
            painter.drawText(15,height*(i+2),f"{(t_max-(step*(i+1)))/10000}")
    if not res:
        painter.drawText(15,580,str(t_min)) #Ultimo desfasado
        painter.drawText(15,75,str(t_max)) #Maximo valor
    else: 
        painter.drawText(15,580,str(t_min/10000)) #Ultimo desfasado
        painter.drawText(15,75,str(t_max/10000)) #Maximo valor
    #painter.drawText(50,595,dias[0]) #Ultimo desfasado
    d_step = int((len(dias)-1)/number_of_lines_w+1)
    #print(len(dias), d_step, number_of_lines_w-2)
    desface=-10
    for i in range(number_of_lines_w):
        if i%2==0:
            painter.drawText(((i+1)*width)+desface,595,dias[d_step*i]) #Ultimo desfasado
        else:
            painter.drawText(((i+1)*width)+desface,75,dias[d_step*i]) #Ultimo desfasado
        #painter.drawLine(((i+1)*width)+desface,75,((i+1)*width)+desface,580)
    return t_min, t_max
    
def draw_graph(data,painter,pen, t_min, t_max):
    pen.setColor(QtGui.QColor('red'))
    pen.setWidth(3)
    painter.setPen(pen)
    if max(data)<1.0:
        data = [x*10000 for x in data]
    #painter.drawLine(46,75,417,580) Limites
    step = int((580-75)/(len(data)+7))
    obj = GraphCoordGen(t_max,t_min,46,525)
    x0,y0 = 46,obj.form_pen(data[0])
    x1,y1 = 46+step,obj.form_pen(data[1])
#    painter.drawLine(x0,340,x1,340) # Mitad falsa
#    painter.drawLine(x0,300,x1,300) # Mitad real
#    painter.drawLine(x0,525,x1,525) # Limite inferior real
    for i in range(2,len(data)):
        painter.drawLine(x0,y0,x1,y1)
        x0,y0,=x1,y1
        tres = obj.form_pen(data[i])
        #print(dias[i],tres,data[i])
        x1,y1 = 46+(step*i), int(tres)
    

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Event Tester")

        self.label = QtWidgets.QLabel()
        canvas = QtGui.QPixmap(448, 600)
        canvas.fill(Qt.black)
        self.label.setPixmap(canvas)
        self.setCentralWidget(self.label)
        self.draw_something()

    def draw_something(self):
        painter = QtGui.QPainter(self.label.pixmap())
        pen = QtGui.QPen()
        pen.setColor(QtGui.QColor('white'))
        painter.setPen(pen)
        #pen.setWidth(10)
       # pen.setColor(QtGui.QColor('red'))
       # painter.setPen(pen)
        lista = doge
        draw_grid(painter,pen)
        draw_name(lista,painter,pen)
        t_min, t_max = draw_graph_grid(lista,painter,pen)
        print(t_min, t_max)
        draw_graph(lista,painter,pen, t_min, t_max)
        painter.end()


app = QtWidgets.QApplication(sys.argv)
window = MainWindow()
window.show()
app.exec_()

