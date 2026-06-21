# Springer Nature / JRTIP LaTeX Manuscript

`access_paper.tex` is the Springer Nature journal-template version of the
manuscript prepared for a Journal of Real-Time Image Processing attempt.

The required Springer Nature class file is `sn-jnl.cls`. Bibliography style
files from the December 2024 Springer Nature journal article template are kept
under `bst/` for upload compatibility, although the current manuscript keeps
references inline in the `.tex` file.

The generated review PDF is `access_paper.pdf`.

Generate the vector PDF figures and high-resolution PNG review copies:

```bash
python3 -m pip install --user -r scripts/plot_requirements.txt
python3 scripts/generate_publication_plots.py
```

Build with a TeX distribution that provides `pdflatex`:

```bash
cd paper/access
pdflatex access_paper.tex
pdflatex access_paper.tex
```

The manuscript includes Springer-style declarations, including funding,
competing interests, data/code availability, and use of generative AI and
AI-assisted technologies.
