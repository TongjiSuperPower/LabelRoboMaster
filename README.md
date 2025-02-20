# 项目介绍

本项目基于 [LabelRoboMaster](https://github.com/MonthMoonBird/LabelRoboMaster) 开源项目进行开发。  

## 改进
我们对 YOLOv11 所需的输入标签格式进行了适配，并对输出格式进行了调整。调整后的格式为：
```
<目标id> [bbox的xywh] [目标各点的x、y归一化坐标]
```
具体使用方法，请参考 [使用教程](https://github.com/MonthMoonBird/LabelRoboMaster/blob/main/README.md)。

## 感谢

感谢[上海交通大学交龙战队](https://github.com/SJTU-RoboMaster-Team) [xinyang](https://github.com/xinyang-go) 、 [哈尔滨工业大学深圳校区南工骁鹰战队](https://space.bilibili.com/1559398123/)  [MonthMoonBird](https://github.com/MonthMoonBird) 提供的开源项目，使我们能够在其基础上进行改进与开发。我们将继续在此基础上进行优化，并分享更多的功能与改进。
