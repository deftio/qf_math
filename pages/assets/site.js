/* ════════════════════════════════════════════════════════════════════
   qf_math docs — shared site chrome.
   Plain vanilla JS, no framework. Injects the header nav and footer
   into every page so there's exactly one source of truth for the
   site title, version, menu, and legal line.

   Version is loaded from version.json so there is no hardcoded
   version string in this file.

   Page skeleton expected in each HTML file:

       <header id="site-header"></header>
       <main class="page-content"><div class="wrapper">...</div></main>
       <footer id="site-footer"></footer>
       <script src="assets/site.js"></script>
   ════════════════════════════════════════════════════════════════════ */

(function () {
    var path    = window.location.pathname;
    var current = path.split('/').pop();
    if (!current) { current = 'index.html'; }

    // Nav items: [href, label, basename for active-link match]
    var nav = [
        ['index.html',                 'Home',         'index.html'],
        ['quickstart.html',           'Quick Start',  'quickstart.html'],
        ['api.html',                   'API',          'api.html'],
        ['algorithms.html',           'Algorithms',   'algorithms.html'],
        ['integration.html',          'Integration',  'integration.html'],
        ['float-math-tradeoffs.html', 'Tradeoffs',    'float-math-tradeoffs.html'],
        ['fr-math.html',              'fr_math',      'fr-math.html'],
        ['https://github.com/deftio/qf_math', 'GitHub', '']
    ];

    // -----------------------------------------------------------------
    // Build header
    // -----------------------------------------------------------------
    var headerEl = document.getElementById('site-header');
    if (headerEl) {
        var navHtml = '';
        for (var i = 0; i < nav.length; i++) {
            var href       = nav[i][0];
            var label      = nav[i][1];
            var basename   = nav[i][2];
            var activeAttr = (current === basename) ? ' class="active"' : '';
            navHtml += '<a href="' + href + '"' + activeAttr + '>' + label + '</a>';
        }

        headerEl.className = 'site-header';
        headerEl.innerHTML =
            '<div class="wrapper">' +
              '<a class="site-title" href="index.html">' +
                'qf_math <span class="site-version" id="site-version"></span>' +
              '</a>' +
              '<nav class="site-nav">' + navHtml + '</nav>' +
            '</div>';
    }

    // -----------------------------------------------------------------
    // Load version from version.json
    // -----------------------------------------------------------------
    try {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', 'version.json', true);
        xhr.onload = function () {
            if (xhr.status === 200 || xhr.status === 0) {
                try {
                    var data = JSON.parse(xhr.responseText);
                    var el = document.getElementById('site-version');
                    if (el && data.version) {
                        el.textContent = 'v' + data.version;
                    }
                } catch (e) { /* malformed JSON */ }
            }
        };
        xhr.send();
    } catch (e) { /* blocked — leave blank */ }

    // -----------------------------------------------------------------
    // Build footer
    // -----------------------------------------------------------------
    var footerEl = document.getElementById('site-footer');
    if (footerEl) {
        footerEl.className = 'site-footer';
        footerEl.innerHTML =
            '<div class="wrapper">' +
              '<span>qf_math &mdash; BSD-2-Clause &mdash; ' +
                '&copy; M. A. Chatterjee</span>' +
              '<span>' +
                '<a href="https://github.com/deftio/qf_math">GitHub</a>' +
                ' &middot; ' +
                '<a href="https://github.com/deftio/fr_math">fr_math</a>' +
              '</span>' +
            '</div>';
    }

    // -----------------------------------------------------------------
    // Lazy-load highlight.js from CDN for code blocks.
    // -----------------------------------------------------------------
    var HLJS_BASE = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0';

    if (document.querySelector('pre code')) {
        var link = document.createElement('link');
        link.rel  = 'stylesheet';
        link.href = HLJS_BASE + '/styles/github.min.css';
        document.head.appendChild(link);

        var script = document.createElement('script');
        script.src = HLJS_BASE + '/highlight.min.js';
        script.onload = function () {
            if (window.hljs) { window.hljs.highlightAll(); }
        };
        document.body.appendChild(script);
    }
})();
