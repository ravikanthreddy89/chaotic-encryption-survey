# IEEE Access LaTeX Manuscript

`access_paper.tex` is the IEEE Access conversion of the maintained
`paper_draft.md` manuscript.

The required IEEE Access class, style, logo, and font assets are included in
this directory from the parent `ACCESS_latex_template_20260513` template.

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

IEEE supplies the final publication history and DOI. Replace those placeholders
before submission if the submission system requires different values.
