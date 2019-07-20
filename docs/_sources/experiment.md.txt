<!-- experiment.md --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 二 5月 28 10:38:45 2019 (+0800)
;; Last-Updated: 六 7月 20 19:57:33 2019 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 3
;; URL: http://wuhongyi.cn -->

# experiment

- 当多重性为 0 时，输出高电平
- 当多重性大于等于 1 时，默认输出低电平，有触发时为高电平

- MSRB bit6为1时，才能有同步指示信号，才能DPM的输出信息，才有FT，VT信息

## 在线监视

修改好参数配置文件 settings.ini

执行
```bash
progfippi
```
写入设置参数

**需要注意的是，当获取采集时候，不允许执行该程序**

在网页中即可查看参数设置情况，scale计数情况




<!-- experiment.md ends here -->
