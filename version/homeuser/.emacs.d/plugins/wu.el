;;; wu.el --- 
;; 
;; Description: 
;; Author: Hongyi Wu(吴鸿毅)
;; Email: wuhongyi@qq.com 
;; Created: 二 12月  9 13:37:24 2014 (+0800)
;; Last-Updated: 日 7月  1 10:54:05 2018 (+0800)
;;           By: Hongyi Wu(吴鸿毅)
;;     Update #: 159
;; URL: http://wuhongyi.cn 


;; latex-mode
(add-to-list 'ac-modes 'latex-mode)
(defun ac-latex-mode-setup()
  (setq ac-sources (append '(ac-source-yasnippet) ac-sources)))
(add-hook 'latex-mode-hook 'ac-latex-mode-setup)

;; ;; Load CEDET.
;; ;; See cedet/common/cedet.info for configuration details.
;; (add-to-list 'load-path"~/.emacs.d/cedet-1.1/common")
;; (load-library "~/.emacs.d/cedet-1.1/common/cedet.el")
;; (require 'cedet)
;; (require 'semantic-ia)
;; ;; Enable EDE (Project Management) features
;; (global-ede-mode 1)
;; (semantic-load-enable-minimum-features)
;; (semantic-load-enable-code-helpers)
;; ;(global-srecode-minor-mode 1)
;; (global-set-key [(f7)] 'speedbar-get-focus);speedbar快捷键
;; (setq speedbar-show-unknown-files t);;可以显示所有目录以及文件
;; (setq dframe-update-speed nil);;不自动刷新，手动 g 刷新  
;; (setq speedbar-update-flag nil)  
;; (setq speedbar-use-images nil);;不使用 image 的方式  
;; (setq speedbar-verbosity-level 0)

;; (global-set-key [f5] 'senator-fold-tag);代码折叠
;; (global-set-key [f6] 'senator-unfold-tag);代码展开
;; (when (and window-system (require 'semantic-tag-folding nil 'noerror))
;;   (global-semantic-tag-folding-mode 1)
;;   (global-set-key (kbd "C-?") 'global-semantic-tag-folding-mode);打开semantic-tag-folding-mode后，用gdb调试时不能点左侧的fringe切换断点了，所以我把C-?定义为semantic-tag-folding-mode的切换键，在gdb调试时临时把semantic-tag-folding关掉
;;   (global-set-key [f5] 'semantic-tag-folding-fold-block);代码折叠
;;   (global-set-key [f6] 'semantic-tag-folding-show-block);代码展开
;;   (global-set-key [C-f5] 'semantic-tag-folding-fold-all)
;;   (global-set-key [C-f6] 'semantic-tag-folding-show-all))

;; ;;代码跳转  
;; (global-set-key [f3] 'semantic-ia-fast-jump)  
;; ;;不提示直接就跳回上次的位置  
;; (global-set-key [C-f3]  
;;                 (lambda ()  
;;                   (interactive)  
;;                   (if (ring-empty-p (oref semantic-mru-bookmark-ring ring))  
;;                       (error "Semantic Bookmark ring is currently empty"))  
;;                   (let* ((ring (oref semantic-mru-bookmark-ring ring))  
;;                          (alist (semantic-mrub-ring-to-assoc-list ring))  
;;                          (first (cdr (car alist))))  
;;                     (if (semantic-equivalent-tag-p (oref first tag)  
;;                                                    (semantic-current-tag))  
;;                         (setq first (cdr (car (cdr alist)))))  
;;                     (semantic-mrub-switch-tags first))))  


;; ;##ecb(Emacs Code Browser);;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; (add-to-list 'load-path "~/.emacs.d/ecb-2.40")
;; (load-file "~/.emacs.d/ecb-2.40/ecb.el")
;; (require 'ecb)
;; (require 'ecb-autoloads)
;; ;;开启ecb用,M-x:ecb-activate
;; ;(setq ecb-auto-activate t);自动启动ecb
;; (setq ecb-auto-activate nil;不启动ecb
;; ecb-tip-of-the-day nil;不显示每日提醒
;; inhibit-startup-message t;不知道什么意思,望各位指导
;; ecb-auto-compatibility-check nil;
;; ecb-version-check nil;
;; )
;; (setq stack-trace-on-error t)
;; (global-set-key [f8] 'ecb-activate) ;;定义F8键为激活ecb
;; (global-set-key [C-f8] 'ecb-deactivate) ;;定义C-F8为停止ecb


;;;; 各窗口间切换
;; (global-set-key [M-left] 'windmove-left)
;; (global-set-key [M-right] 'windmove-right)
;; (global-set-key [M-up] 'windmove-up)
;; (global-set-key [M-down] 'windmove-down)
;;;; 使某一ecb窗口最大化
;; (define-key global-map "\C-c1" 'ecb-maximize-window-directories)
;; (define-key global-map "\C-c2" 'ecb-maximize-window-sources)
;; (define-key global-map "\C-c3" 'ecb-maximize-window-methods)
;; (define-key global-map "\C-c4" 'ecb-maximize-window-history)
;;;; 恢复原始窗口布局
;; (define-key global-map "\C-c`" 'ecb-restore-default-window-sizes)


;; (require 'eassist)  ;;这个工具不错   ;h, cpp文件跳转函数, 支持即时按键选择 http://www.emacswiki.org/emacs/EAssist
;; (require 'sourcepair) ;;头文件导航
;; (define-key global-map "\C-cz" 'sourcepair-jump-to-headerfile) ;;跳转到头文件的设置
;; (setq sourcepair-source-path    '( "." "../*" "../../*" ))
;; (setq sourcepair-header-path    '( "." "include" "../include" "../*" "../../*"))
;; (setq sourcepair-recurse-ignore '( "CVS"  "Debug" "Release" ))


;; 放大字体: Ctrl-x Ctrl-+ 或 Ctrl-x Ctrl-=
;; 缩小字体: Ctrl-x Ctrl–
;; 重置字体: Ctrl-x Ctrl-0
(global-set-key [f7] 'text-scale-increase)
(global-set-key [f8] 'text-scale-decrease)
;; (setq cjk-font-size 16)
;; (setq ansi-font-size 16)
;; (global-set-key '[C-wheel-up] 'increase-font-size)
;; (global-set-key '[C-wheel-down] 'decrease-font-size)
;; (global-set-key (kbd "<C-wheel-up>") 'text-scale-increase)
;; (global-set-key (kbd "<C-wheel-down>") 'text-scale-decrease)


;;显示/隐藏工具栏 方便调试
(global-set-key [f1] 'tool-bar-mode)
;;显示/隐藏菜单栏 ;; M-x menu-bar-open
(global-set-key [C-f1] 'menu-bar-mode)

;;设置编译命令;保存所有文件
(defun du-onekey-save ()
  "Save buffers"
  (interactive)
  (save-some-buffers t)
  );;(switch-to-buffer-other-window "*compilation*")
  ;;(compile compile-command)  
;; (setq-default compile-command "make")    
;; (global-set-key [C-f12] 'compile)
 (global-set-key [f12] 'du-onekey-save)

;;目的是开一个shell的小buffer，用于更方便地测试程序(也就是运行程序了)，我经常会用到。
;;f11就是另开一个buffer然后打开shell，C-f13则是在当前的buffer打开shell,shift+f13清空eshell
(defun open-shell-other-buffer ()
  "Open shell in other buffer"
  (interactive)
  (split-window-horizontally);;horizontally vertically
  (shell))

(global-set-key [(f11)] 'open-shell-other-buffer)
(global-set-key [C-f11] 'shell)


;;设置F9为撤销
(global-set-key [f9] 'undo)
;;设置F11快捷键指定Emacs 的日历系统
;;(global-set-key [C-f10] 'calendar) 
;;设置C-F12 快速察看日程安排
;;(global-set-key [f10] 'list-bookmarks)
;;关闭当前缓冲区
(global-set-key [C-f10] 'kill-this-buffer)
;;关闭当前窗口
(global-set-key [f10] 'delete-window)

;##GDB;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(load-library "multi-gud.el")
(load-library "multi-gdb-ui.el")
(setq gdb-many-windows t)
(global-set-key [f2] 'gdb);gdb
;;(global-set-key [f4] 'gdb-many-windows);gdb-many-windows快捷键[F4]
;;(setq gdb-use-separate-io-buffer t) ; 不需要"IO buffer"时，则设为nil



(load-library "lammps.el")

;;git-emacs;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(require 'git-emacs)
(global-set-key [f4] 'git-status);


(require 'window-numbering)
(window-numbering-mode 1)
(winner-mode 1)
(global-set-key (kbd "C-x 4 u") 'winner-undo)
(global-set-key (kbd "C-x 4 r") 'winner-redo)


;;cmake;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(require 'cmake-mode)




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; wu.el ends here
