# Configuration file for Sphinx,
# see https://www.sphinx-doc.org/en/master/usage/configuration.html

from subprocess import check_output

# -- General configuration -----------------------------------------------------

extensions = [
    'sphinx_last_updated_by_git',
]

master_doc = 'index'

project = u'SoundScape Renderer'
copyright = u'2018, SSR Team'

try:
    release = check_output(['git', 'describe', '--tags', '--always'])
    release = release.decode().strip()
except Exception:
    release = '<unknown>'

language = 'en'

try:
    today = check_output(['git', 'show', '-s', '--format=%ad', '--date=short'])
    today = today.decode().strip()
except Exception:
    today = '<unknown date>'

exclude_patterns = ['_build']

highlight_language = 'sh'

# -- Options for HTML output ---------------------------------------------------

html_theme = 'insipid'
html_favicon = 'favicon.svg'

html_domain_indices = False
html_use_index = False
html_show_copyright = False
html_copy_source = False
html_add_permalinks = '\N{SECTION SIGN}'

# -- Options for LaTeX output --------------------------------------------------

latex_elements = {
    'babel': '\\usepackage[english]{babel}',
    #'preamble': '',
}
latex_documents = [
   ('index', 'SoundScapeRenderer.tex', project, u'SSR Team', 'manual'),
]
