# Springer / JRTIP LaTeX Manuscript

`access_paper.tex` is the Springer journal-template version of the manuscript
prepared for a Journal of Real-Time Image Processing attempt.

The source package uses the Springer macro package supplied by the journal
guidelines: `svjour3.cls`, `svglov3.clo`, and the `sp*.bst` bibliography style
files under `bst/`. The manuscript is built in two-column mode to match the
JRTIP LaTeX guidance and keep the first submission within the 12-page target.
References are currently kept inline in the `.tex` file.

The generated review PDF is `access_paper.pdf`.

Generate the vector PDF figures and high-resolution PNG review copies:

```bash
python3 -m pip install --user -r scripts/plot_requirements.txt
python3 scripts/generate_publication_plots.py
```

Build with a TeX distribution that provides `pdflatex`:

```bash
cd paper/access
pdflatex -interaction=nonstopmode access_paper.tex
pdflatex -interaction=nonstopmode access_paper.tex
```

The manuscript includes Springer-style declarations, including funding,
competing interests, data/code availability, and use of generative AI and
AI-assisted technologies.
