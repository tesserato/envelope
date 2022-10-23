from os import walk

upper_body = ''' 
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Signal Envelope live examples</title>
</head>
<body>
<h1>Signal Envelope Examples</h1>
<a href="https://github.com/tesserato/envelope">Github Repository</a>
<a href="https://pypi.org/project/signal-envelope/">PyPI Module</a>
<a href="https://arxiv.org/abs/2009.02860">arXiv Paper</a>
'''

lower_body = '''
</body>
</html>
'''

files = walk("./site/")
filenames = [f for f in files][0][2]

html = ""
html += upper_body

for fn in filenames:
    if fn != "index.html":
        file = open("./site/" + fn, "r")
        html += f"<h2>{fn.replace('.html', '')}</h2>\n"
        html += file.read()
        print(fn)
        file.close()

html += lower_body

file = open("./site/index.html", "w")
file.write(html)
file.close()
