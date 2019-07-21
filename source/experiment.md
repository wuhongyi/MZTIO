<!-- experiment.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 二 5月 28 10:38:45 2019 (+0800)
;; Last-Updated: 日 7月 21 21:19:02 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 6
;; URL: http://wuhongyi.cn -->

# experiment

About multiplicity output in RJ45 in PKU firmware

- when setting multiplicity==0, output high level
- when setting multiplicity>=1, the default output is low level, and it is high when triggered.



**When the MSRB bit 6 is 1**
- the synchronization indication signal can be obtained
- have the DPMFULL output information
- have back plane FT, VT information


- MSRB bit6为1时，才能有同步指示信号，才能DPM的输出信息，才有FT，VT信息

## online monitor

After modifying the parameter configuration file ```settings.ini```, you need to run the following program to modify the register settings.

```bash
./progfippi
```

**It should be noted that the program is not allowed to be executed when DAQ running**

You can view the parameters settings in the web page, and the scaler counter and so on.




----

## experiment mode

We will provide a common combination of firmware and software for the following four types of experiments.

## in beam gamma

**designing. . .**


## beta decay

**designing. . .**


## nuclear reaction

**designing. . .**


## Super heavy nucleus

**designing. . .**




<!-- experiment.md ends here -->
