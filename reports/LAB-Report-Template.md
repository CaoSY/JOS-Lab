---
# export_on_save:
#  ebook: "html"

ebook:
  title: "JOS Lab Report"
  author: "Shuyang Cao"
  language: English
  include_toc: true
  html:
    cdn: true
---

```python {cmd=true hide=true run_on_save=true output="html" id="global"}
import time
from functools import reduce

# global variables
# labnumber = 1
author = "Shuyang Cao"
# createDate = "Mar 8 2018"
# createTime = "23:18:00"

KEYS = locals().keys()
var_tobe_check = ['labnumber', 'createDate', 'createTime']
not_in_KEYS = [c not in KEYS for c in var_tobe_check]
if reduce(lambda x,y: x or y, not_in_KEYS) :
    for (varTmp, not_in_key) in zip(var_tobe_check, not_in_KEYS) :
        print('<p style="color:red;font-size:4em">{_var_} is not defined</p>'.format(_var_=varTmp))
    raise NameError('At least ones of labnumber, createDate and createTime is not defined')

title = "Lab {_number} Report".format(_number=labnumber)
currentTime = time.localtime()
updateDate = time.strftime("%b %d %Y", currentTime)
updateTime = time.strftime("%H:%M:%S", currentTime)
```

```python {cmd=true hide=true run_on_save=true output="html" continue="global"}

ctStyle = 'display:inline-block;width:50%;text-align:left'
ctSpan = '<span style="{_style}">Created: {_date} {_time}</span>'
ctSpan = ctSpan.format(_style=ctStyle, _date=createDate, _time=createTime)

mtStyle = 'display:inline-block;width:50%;text-align:right'
mtSpan = '<span style="{_style}">Last Updated: {_date} {_time}</span>'
mtSpan = mtSpan.format(_style=mtStyle, _date=updateDate, _time=updateTime)

header = '<p>{_ct}{_mt}</p>'.format(_ct=ctSpan, _mt=mtSpan)


titleStyle = "font-style:italic; font-family:Times; font-size:4em"
titleH1 = '<h1 style="{_style}"><center>{_title}</center></h1>'
titleH1 = titleH1.format(_style=titleStyle, _title=title)

authorStyle = "font-style:italic; font-family:Times; font-size:1.2em"

authorDiv = '<div style="{_style}"><center>{_author}</center><center>{_updateDate}</center></div>'
authorDiv = authorDiv.format(_style=authorStyle, _author=author, _updateDate=updateDate)

print(header)
print(titleH1)
print(authorDiv)
```

[TOC]

## Grade

Finally, we got our grade.

```bash
```

```python {cmd=true hide=true run_on_save=true output="html" continue="global"}

footerStyle = "width:100%;text-align:center;font-family:Times"
footerTemplate = '<footer style="{_style}">End of {_title}<br/>Email: <a mailto="caoshuyang1996@pku.edu.cn">caoshuyang@pku.edu.cn</a> GitHub: <a href="https://github.com/CaoSY/JOS-Lab" title="JOS Lab">JOS-Lab</a></footer>'

# print footer
print(footerTemplate.format(_style=footerStyle, _title=title))
```