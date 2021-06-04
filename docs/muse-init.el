(muse-derive-style "othello-xhtml" "xhtml"
									 :style-sheet "<style type=\"text/css\">
<include file=\"common.css\" markup=\"nil\">
</style>")

;; add a <source> tag, which use source-highlight
(defun muse-publish-source-tag (beg end attrs)
  (let ((lang (muse-publish-get-and-delete-attr "lang" attrs))
        (style (muse-style-element :base))
         hcodes)
    (unless style
      (setq style (car muse-publishing-current-style)))
    (if (string-equal style "pdf") (setq style "latexcolor"))
    (if lang
        (progn
          (muse-publish-command-tag
           beg end
           (cons (cons "interp" (format "echo \"<literal><pre class=\"example\">
$(source-highlight -s %s -f %s)
</pre></literal>\"" lang style))
                 attrs))
          (muse-publish-mark-read-only beg (point)))
      (error "No lang attribute specified in <source> tag"))))

(add-to-list
 'muse-publish-markup-tags
 '("source" t t nil muse-publish-source-tag))

