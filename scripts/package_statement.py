"""Build the self-contained local VS Code statement extension using only Python stdlib."""
import argparse
import json
from pathlib import Path
from xml.sax.saxutils import escape
import zipfile


def build(output=None):
    root = Path(__file__).resolve().parent.parent
    source = root / 'scripts/statement-extension'
    package = json.loads((source / 'package.json').read_text(encoding='utf-8'))
    version = package['version']
    output = Path(output).resolve() if output else root / f'docs/releases/zoi-statement-{version}.vsix'
    output.parent.mkdir(parents=True, exist_ok=True)
    files = [p for p in source.rglob('*') if p.is_file() and not {'node_modules', '.git'}.intersection(p.relative_to(source).parts) and p.name != 'package-lock.json']
    required = ['extension.cjs', 'core.cjs', 'trash.cjs', 'renderer.js', 'style.css', 'vendor/dompurify/dist/purify.min.js', 'vendor/markdown-it/dist/browser/markdown-it.umd.min.js', 'vendor/katex/dist/katex.min.js', 'vendor/pdfjs-dist/build/pdf.min.mjs', 'vendor/pdfjs-dist/build/pdf.worker.min.mjs']
    for name in required:
        if not (source / name).is_file():
            raise ValueError('Missing runtime asset: ' + name)
    if any(p.is_symlink() for p in files):
        raise ValueError('Symlinks cannot enter the extension')
    manifest = f'''<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
<Metadata><Identity Language="en-US" Id="{package['name']}" Version="{version}" Publisher="{package['publisher']}"/>
<DisplayName>{escape(package['displayName'])}</DisplayName><Description xml:space="preserve">{escape(package['description'])}</Description>
<Tags>competitive programming,CPH</Tags><Categories>Other</Categories><GalleryFlags/>
<Properties><Property Id="Microsoft.VisualStudio.Code.Engine" Value="{package['engines']['vscode']}"/><Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value=""/><Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value=""/></Properties></Metadata>
<Installation><InstallationTarget Id="Microsoft.VisualStudio.Code"/></Installation><Dependencies/>
<Assets><Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true"/></Assets></PackageManifest>'''
    content_types = '''<?xml version="1.0" encoding="utf-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="json" ContentType="application/json"/><Default Extension="vsixmanifest" ContentType="text/xml"/><Default Extension="js" ContentType="application/javascript"/><Default Extension="mjs" ContentType="application/javascript"/><Default Extension="cjs" ContentType="application/javascript"/><Default Extension="css" ContentType="text/css"/><Default Extension="woff2" ContentType="font/woff2"/><Default Extension="wasm" ContentType="application/wasm"/><Default Extension="md" ContentType="text/markdown"/></Types>'''
    partial = output.with_suffix('.vsix.partial')
    try:
        with zipfile.ZipFile(partial, 'w', zipfile.ZIP_DEFLATED) as z:
            z.writestr('extension.vsixmanifest', manifest)
            z.writestr('[Content_Types].xml', content_types)
            for p in files:
                name = p.relative_to(source).as_posix()
                if name == 'package.json':
                    public = {k: v for k, v in package.items() if k != 'devDependencies'}
                    z.writestr('extension/package.json', json.dumps(public, ensure_ascii=False, indent=2))
                else:
                    z.write(p, 'extension/' + name)
        with zipfile.ZipFile(partial) as z:
            assert z.testzip() is None
            assert all('extension/' + name in z.namelist() for name in required)
            assert not any('node_modules' in name for name in z.namelist())
        partial.replace(output)
    finally:
        partial.unlink(missing_ok=True)
    print(f'[PASS] VSIX: {output} ({len(files)} assets, {output.stat().st_size} bytes)')
    return output


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output')
    args = parser.parse_args()
    build(args.output)
