from controller import Controller
import time

contr = Controller()
# contr.pid(-0.3)
# time.sleep(3)
contr.pid(-0.3)
time.sleep(5)
# contr.pid(-0.3)
# time.sleep(1)
# contr.pid(0.3)
# time.sleep(1)
# contr.forward(10)

# contr.turnoff()
print("end")

