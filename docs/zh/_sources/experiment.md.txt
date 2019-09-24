<!-- experiment.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 二 5月 28 10:38:45 2019 (+0800)
;; Last-Updated: 二 9月 24 20:21:28 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 7
;; URL: http://wuhongyi.cn -->

# 实验

关于 PKU 固件从前面板网口 RJ45 输出多重性选择的结果

- 当设置 multiplicity==0, 输出高电平
- 当设置 multiplicity>=1, 默认输出低电平，只有满足多重性条件时才有高电平。

**MSRB bit6为1时**
- 才能有同步指示信号
- 才能 DPM 的输出信息
- 才有FT，VT信息

## 在线监视

在修改参数文件 ```settings.ini``` 之后，你需要运行以下程序来修改寄存器的设置。

```bash
./progfippi
```

**需要注意的是，运行 DAQ 时不允许执行该程序**

您可以在网页中查看参数设置，以及计数器等情况。


----

## 实验模式

我们将为以下四种类型的实验提供固件和软件的通用组合。

## 在束 gamma 谱学

**设计中. . .**


## beta 衰变

**设计中. . .**


## 核反应

**设计中. . .**


## 超重核

**设计中. . .**




<!-- experiment.md ends here -->
