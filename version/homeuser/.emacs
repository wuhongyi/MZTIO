;; .emacs

;; VERSION
;; https://github.com/auto-complete
;; https://jblevins.org/projects/markdown-mode/
;;20180629 yasnippet.el 没有旧版 yasnippet-bundle.el 好
;;20180630 UPDATE auto-complete.el 1.3.1 -> 1.5.1
;;20180630 UPDATE fuzzy.el -> 0.2
;;20180630 UPDATE popup.el 0.4 -> 0.5.3
;;20180630 修改 auto-complete-config.el 扩展自动补全/yasnippet.el   yasnippet-bundle.el不需要修改
;;20180701 UPDATE header2.el 2014 - 2018
;;20180701 修改 header2.el 支持外部设置邮箱、网址
;;20180701 UPDATE markdown-mode.el 2.0 -> 2.3
;;20180701 UPDATE color-theme.el 6.5.4 -> 6.6.0

;;------------------------------------------------------------------------------

;;F1 显示/隐藏工具栏 C-F1 显示/隐藏菜单栏
;;F2 gdb
;;F3 代码转跳  C-F3 不提示跳回
;;F4 git-status
;;F7 放大字体
;;F8 缩小字体
;;F9 undo
;;F10 delete-window   C-F10 kill-this-buffer
;;F11 open-shell-other-buffer  C-F11 当前窗口打开shell
;;F12 onekey-save


;;    设置个人信息      -----用户需要修改-----
(setq user-full-name "Hongyi Wu(吴鸿毅)");;函数
(setq user-mail-address "wuhongyi@qq.com");;变量
(setq user-url-address "http://wuhongyi.cn");;变量
(add-hook 'after-init-hook (lambda ()
(setq frame-title-format "%b - Hongyi Wu @ Peking University")))
(add-to-list 'load-path "~/.emacs.d/plugins")
(add-to-list 'load-path "~/.emacs.d/git-emacs")
(add-to-list 'load-path "~/.emacs.d/lammps")
(add-to-list 'load-path "/usr/local/MATLAB/R2012a/bin") ; matlab path
(setq user-path-acdict "~/.emacs.d/plugins/ac-dict")
(setq user-path-snippets "~/.emacs.d/snippets")
;; ac-clang-flags是头文件的目录，根据系统的不同可能你的头文件目录也会不同，列出系统中所有的头文件目录方法是：
;; $ echo "" | g++ -v -x c++ -E -  
(setq user-path-ac-clang-include 
      "  
 /usr/lib/gcc/x86_64-unknown-linux-gnu/4.9.4/../../../../include/c++/4.9.4
 /usr/lib/gcc/x86_64-unknown-linux-gnu/4.9.4/../../../../include/c++/4.9.4/x86_64-unknown-linux-gnu
 /usr/lib/gcc/x86_64-unknown-linux-gnu/4.9.4/../../../../include/c++/4.9.4/backward
 /usr/lib/gcc/x86_64-unknown-linux-gnu/4.9.4/include
 /usr/local/include
 /usr/lib/gcc/x86_64-unknown-linux-gnu/4.9.4/include-fixed
 /usr/include
 /opt/root61206/include
 /home/wuhongyi/CodeProject/inc
 /home/wuhongyi/CodeProject/inc/wu_CLHEP
 /home/wuhongyi/CodeProject/G4_inc
 /home/wuhongyi/CodeProject/ROOT_inc
 /opt/Geant4/geant41004p02/include/Geant4
    ")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                            以下部分用户无需修改
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;markdown-mode
(autoload 'markdown-mode "markdown-mode.el"
"Major mode for editing Markdown files" t)
(setq auto-mode-alist
(cons '(".markdown\\'" . markdown-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".md\\'" . markdown-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".ejs\\'" . markdown-mode) auto-mode-alist))



;; For my language code setting (UTF-8)
(set-language-environment "UTF-8")



;; 实现全屏效果
;; (defun my-fullscreen ()
;; (interactive)
;; (x-send-client-message
;; nil 0 nil "_NET_WM_STATE" 32
;; '(2 "_NET_WM_STATE_FULLSCREEN" 0))
;; )
;; (my-fullscreen)

;;CUA矩形块操作
(cua-mode 1)
;;显示匹配的括号
(show-paren-mode t) 
;;光标显示为一竖线
(setq-default cursor-type 'bar)
;;鼠标指针避光标
;; (mouse-avoidance-mode 'animate)
;; (mouse-avoidance-mode 'cat-and-mouse)
;;开启语法高亮。
(global-font-lock-mode 1)
;;高亮显示区域选择
(transient-mark-mode t)
;;不生成临时文件
(setq-default make-backup-files nil)
;;显示行列号
(setq column-number-mode t) 
(setq line-number-mode t)
;;允许emacs和外部其他程序的粘贴
(setq x-select-enable-clipboard t)
;;使用鼠标中键可以粘贴
(setq mouse-yank-at-point t)
;; 自动的在文件末增加一新行
(setq require-final-newline t)
;; 当光标在行尾上下移动的时候，始终保持在行尾。
;; (setq track-eol t);这个会导致tab失效
(auto-image-file-mode t) ;让Emacs可以直接打开、显示图片
(tool-bar-mode 0);默认隐藏工具栏
;; 去掉滚动条
(set-scroll-bar-mode nil)
;;设置home键指向buffer开头，end键指向buffer结尾  
(global-set-key [home] 'beginning-of-buffer)  
(global-set-key [end] 'end-of-buffer)

;; move window (Shift + cursor)
(windmove-default-keybindings)
(setq windmove-wrap-around t)


;;代码折叠
(load-library "hideshow")
(add-hook 'java-mode-hook 'hs-minor-mode)
(add-hook 'perl-mode-hook 'hs-minor-mode)
(add-hook 'php-mode-hook 'hs-minor-mode)
(add-hook 'emacs-lisp-mode-hook 'hs-minor-mode)
(add-hook 'c++-mode-hook 'hs-minor-mode)
(add-hook 'c-mode-hook 'hs-minor-mode)
(add-hook 'matlab-mode-hook 'hs-minor-mode)
(add-hook 'fortran-mode-hook 'hs-minor-mode)
(add-hook 'f90-mode-hook 'hs-minor-mode)
(add-hook 'markdown-mode-hook 'hs-minor-mode)
(add-hook 'cmake-mode-hook 'hs-minor-mode)
(add-hook 'vhdl-mode-hook 'hs-minor-mode)
(add-hook 'verilog-mode-hook 'hs-minor-mode)



;;color
(require 'color-theme)
(color-theme-initialize)
;;(color-theme-subtle-hacker)
;;(color-theme-xemacs)
;;(color-theme-wheat)
;;(color-theme-calm-forest)
;;(color-theme-matrix)
;;(color-theme-robin-hood)
(color-theme-euphoria);;颜色很好
;;(color-theme-arjen)
;;(color-theme-jsc-dark)



;;(display time)======
(setq display-time-24hr-format t);;24 hour time format
(display-time-mode 1);;Display current time on minibuffer
(setq display-time-day-and-data t);;Display data and day

;;; uncomment this line to disable loading of "default.el" at startup
;; (setq inhibit-default-init t)

;; enable visual feedback on selections
;(setq transient-mark-mode t)


;; default to unified diffs
(setq diff-switches "-u")

;; always end a file with a newline
;(setq require-final-newline 'query)

;;; uncomment for CJK utf-8 support for non-Asian users
;; (require 'un-define)
(custom-set-variables
  ;; custom-set-variables was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 '(inhibit-startup-screen t))
(custom-set-faces
  ;; custom-set-faces was added by Custom.
  ;; If you edit it by hand, you could mess it up, so be careful.
  ;; Your init file should contain only one such instance.
  ;; If there is more than one, they won't work right.
 )


;;显示行数
(require 'linum)    
;;; show line numbers in buffer  
;;; run M-x linum-mode    
;;; show line numbers in all buffers  
;;; run M-x global-linum-mode    
(global-linum-mode t)  




;;自动补全
(require 'auto-complete)
(require 'auto-complete-config)
(add-to-list 'ac-dictionary-directories user-path-acdict)
(ac-config-default)


(require 'auto-complete-clang) 
;; (setq ac-clang-auto-save t) 
(setq ac-auto-start t);;nil  t
(setq ac-quick-help-delay 0.5);;默认0.5
(ac-set-trigger-key "TAB")
;; (define-key ac-mode-map  [(control tab)] 'auto-complete)
(defun my-ac-config ()
  (setq ac-clang-flags  
	(mapcar(lambda (item)(concat "-I" item))  
	       (split-string  
		user-path-ac-clang-include
		))) 
  (setq-default ac-sources '(ac-source-abbrev ac-source-dictionary ac-source-words-in-same-mode-buffers))
  (add-hook 'emacs-lisp-mode-hook 'ac-emacs-lisp-mode-setup)
  ;;(add-hook 'c-mode-common-hook 'ac-cc-mode-setup)
  (add-hook 'ruby-mode-hook 'ac-ruby-mode-setup)
  (add-hook 'css-mode-hook 'ac-css-mode-setup)
  (add-hook 'auto-complete-mode-hook 'ac-common-setup)
  (global-auto-complete-mode t))
(defun my-ac-cc-mode-setup ()
  (setq ac-sources (append '(ac-source-clang ac-source-yasnippet) ac-sources)))
(add-hook 'c-mode-common-hook 'my-ac-cc-mode-setup)
;; ac-source-gtags
(my-ac-config);;windows下注释该行

;;------------------------------------------------------------------------------

;;模板调用
(require 'yasnippet-bundle) 
(setq yas/root-directory user-path-snippets)
(yas/load-directory yas/root-directory)
;; (require 'yasnippet)
;; (yas/load-directory user-path-snippets)
;; (yas-global-mode 1)
;; (yas-reload-all)
;; (add-hook 'vhdl-mode-hook 'yas-minor-mode)
;; (add-hook 'c++-mode-hook 'yas-minor-mode)
;; (add-hook 'verilog-mode-hook 'yas-minor-mode)




;;加载header2.el文件,自动添加文件头
(require 'header2)
(autoload 'auto-update-file-header "header2")
(add-hook 'write-file-hooks 'auto-update-file-header)
(autoload 'auto-make-header "header2")
(add-hook 'emacs-lisp-mode-hook 'auto-make-header)
(add-hook 'c-mode-common-hook   'auto-make-header) 
(add-hook 'c++-mode-common-hook   'auto-make-header)
(add-hook 'text-mode-hook   'auto-make-header)
(add-hook 'matlab-mode-hook 'auto-make-header)
(add-hook 'latex-mode-hook 'auto-make-header)
(add-hook 'fortran-mode-hook 'auto-make-header)
(add-hook 'f90-mode-hook 'auto-make-header)
(add-hook 'python-mode-hook 'auto-make-header)
(add-hook 'Markdown-mode-hook 'auto-make-header)
(add-hook 'cmake-mode-hook 'auto-make-header)
(add-hook 'vhdl-mode-hook 'auto-make-header)
(add-hook 'verilog-mode-hook 'auto-make-header)



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
;;set matlab-emacs environment   
(require 'matlab-load)  
(autoload 'run-octave "octave-inf" nil t)                         ;special  
(autoload 'matlab-mode "matlab" "Enter MATLAB mode." t)  
(setq auto-mode-alist (cons '("\\.m\\'" . matlab-mode) auto-mode-alist))  
(autoload 'matlab-shell "matlab" "Interactive MATLAB mode." t)  
  
(setq matlab-indent-function-body t)    ; if you want function bodies indented  
(setq matlab-verify-on-save-flag nil)   ; turn off auto-verify on save  
(defun my-matlab-mode-hook ()  
  (setq fill-column 76))        ; where auto-fill should wrap  
(add-hook 'matlab-mode-hook 'my-matlab-mode-hook)  
(defun my-matlab-shell-mode-hook ()  
  '())  
(add-hook 'matlab-shell-mode-hook 'my-matlab-shell-mode-hook)  
(global-font-lock-mode t)  
                    ;  To get hilit19 support try adding:  
(require 'tlc)  
(autoload 'tlc-mode "tlc" "tlc Editing Mode" t)  
(add-to-list 'auto-mode-alist '("\\.tlc$" . tlc-mode))  
(setq tlc-indent-function t) 
;;已经在 .bnshrc 中加入了 matlab 路径了还是不好使，所以在root权限下执行以下命令 ln -s /usr/local/MATLAB/R2012a/bin/matlab /usr/bin/matlab 
;;在/usr/bin目录下链接一个matlab的shell脚本




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(require 'root-help)
;; (defun root-c++-mode-hook ()
;;   "Hook for C++ mode - binding ROOT functions"
;;   (define-key c++-mode-map "\C-crc"  'root-class)
;;   (define-key c++-mode-map "\C-crh"  'root-header-skel)
;;   (define-key c++-mode-map "\C-crs"  'root-source-skel)
;;   (define-key c++-mode-map "\C-cri"  'root-include-header)
;;   (define-key c++-mode-map "\C-crm"  'root-main)
;;   (define-key c++-mode-map "\C-crl"  'root-insert-linkdef)
;;   (define-key c++-mode-map "\C-crp"  'root-insert-pragma)
;;   (define-key c++-mode-map "\C-crx"  'root-shell)
;;   (define-key c++-mode-map "\C-crg"  'root-send-line-to-root)
;;   (define-key c++-mode-map "\C-crr"  'root-send-region-to-root)
;;   (define-key c++-mode-map "\C-crb"  'root-send-buffer-to-root)
;;   (define-key c++-mode-map "\C-crf"  'root-execute-file))
;; (add-hook 'c++-mode-hook 'root-c++-mode-hook)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;使用C-c C-c编译。
;; (add-hook 'LaTeX-mode-hook
;;           #'(lambda ()
;;               (add-to-list 'TeX-command-list '("XeLaTeX" "%`xelatex%(mode)%' %t" TeX-run-TeX nil t))
;;               (setq TeX-command-default "XeLaTeX") ;; 设定默认编译命令为XeLaTeX
;;               (setq TeX-view-program-selection '((output-pdf "Evince")))
;; ;;打开自动补全
;; (auto-complete-mode 1)
;; ;;启动mathmode，你也可以不用。
;; (LaTeX-math-mode 1)
;; ;;打开outlinemode
;; (outline-minor-mode 1)


;; ))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 26.1 冲突
;; Use this mode for editing files in the dot-language (www.graphviz.org and http://www.research.att.com/sw/tools/graphviz/).
;; To use graphviz-dot-mode, add
;; (load-file "~/.emacs.d/plugins/graphviz-dot-mode.el")
;; The graphviz-dot-mode will do font locking, indentation, preview of graphs
;; and eases compilation/error location. There is support for both GNU Emacs
;; and XEmacs.
;; Font locking is automatic, indentation uses the same commands as
;; other modes, tab, M-j and C-M-q. Insertion of comments uses the
;; same commands as other modes, M-; . You can compile a file using
;; M-x compile or C-c c, after that M-x next-error will also work.
;; There is support for viewing an generated image with C-c p.


(setq auto-mode-alist
(cons '(".h\\'" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".hh\\'" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".hpp\\'" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".C\\'" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".cc\\'" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".cpp\\'" . c++-mode) auto-mode-alist))


(setq auto-mode-alist
(cons '(".mqh" . c++-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".mq5" . c++-mode) auto-mode-alist))

(setq auto-mode-alist
(cons '(".v\\'" . verilog-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vh\\'" . verilog-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".verilog\\'" . verilog-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vlg\\'" . verilog-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vt\\'" . verilog-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vhd\\'" . vhdl-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vhdl\\'" . vhdl-mode) auto-mode-alist))
(setq auto-mode-alist
(cons '(".vht\\'" . vhdl-mode) auto-mode-alist))

;;为了加快启动速度，配置放在以下文件夹
(load "wu.el")
