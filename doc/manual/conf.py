# Configuration file for Sphinx,
# see https://www.sphinx-doc.org/en/master/usage/configuration.html

from subprocess import check_output

# -- General configuration -----------------------------------------------------

extensions = [
    'sphinx_last_updated_by_git',
]

master_doc = 'index'

project = u'SoundScape Renderer'
copyright = u'2021, SSR Team'

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

linkcheck_ignore = [
    r'http://localhost:\d+',
]

# -- Options for HTML output ---------------------------------------------------

html_theme = 'insipid'
html_favicon = 'favicon.svg'

html_domain_indices = False
html_use_index = False
html_show_copyright = False
html_copy_source = False
html_permalinks_icon = '\N{SECTION SIGN}'

# -- Options for LaTeX output --------------------------------------------------

latex_elements = {
    'babel': '\\usepackage[english]{babel}',
    #'preamble': '',
}
latex_documents = [
   ('index', 'SoundScapeRenderer.tex', project, u'SSR Team', 'manual'),
]

def gh_link_role(rolename, rawtext, text, lineno, inliner,
                     options={}, content=()):
    from docutils import nodes, utils
    github_url = 'https://github.com/SoundScapeRenderer/ssr'
    blob_url = github_url + '/blob/' + release
    base_url = blob_url + '/%s'
    text = utils.unescape(text)
    full_url = base_url % text
    pnode = nodes.reference(internal=False, refuri=full_url)
    pnode += nodes.literal(text, text, classes=['file'])
    return [pnode], []

def setup(app):
    app.add_role('gh-link', gh_link_role)
