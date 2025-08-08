#!/usr/bin/env python3
"""
PDF Splitter Tool

Splits large PDF files into smaller chunks of less than 100 pages each.
Useful for processing large technical documents while maintaining readability.

Usage:
    python pdf_splitter.py <input_directory>
    
Example:
    python pdf_splitter.py /workspaces/openfoam-mcp-server/RDE/data/
"""

import os
import sys
import argparse
from pathlib import Path
from PyPDF2 import PdfReader, PdfWriter


def split_pdf(input_path, output_dir, max_pages=100):
    """
    Split a PDF file into smaller chunks.
    
    Args:
        input_path (str): Path to input PDF file
        output_dir (str): Directory to save split PDFs
        max_pages (int): Maximum pages per output file
        
    Returns:
        list: Paths of created split files
    """
    try:
        # Read the input PDF
        reader = PdfReader(input_path)
        total_pages = len(reader.pages)
        
        if total_pages <= max_pages:
            print(f"✓ {os.path.basename(input_path)}: {total_pages} pages (no split needed)")
            return []
        
        print(f"📄 Splitting {os.path.basename(input_path)}: {total_pages} pages")
        
        # Create output directory if it doesn't exist
        os.makedirs(output_dir, exist_ok=True)
        
        # Calculate number of parts needed
        num_parts = (total_pages + max_pages - 1) // max_pages
        
        split_files = []
        base_name = Path(input_path).stem
        
        for part in range(num_parts):
            start_page = part * max_pages
            end_page = min((part + 1) * max_pages, total_pages)
            
            # Create new PDF writer
            writer = PdfWriter()
            
            # Add pages to this part
            for page_num in range(start_page, end_page):
                writer.add_page(reader.pages[page_num])
            
            # Generate output filename
            output_filename = f"{base_name}_part{part+1:02d}_pages{start_page+1}-{end_page}.pdf"
            output_path = os.path.join(output_dir, output_filename)
            
            # Write the split PDF
            with open(output_path, 'wb') as output_file:
                writer.write(output_file)
            
            split_files.append(output_path)
            print(f"  ✓ Created: {output_filename} ({end_page - start_page} pages)")
        
        return split_files
        
    except Exception as e:
        print(f"❌ Error splitting {os.path.basename(input_path)}: {str(e)}")
        return []


def analyze_pdf(pdf_path):
    """
    Analyze PDF file and return basic information.
    
    Args:
        pdf_path (str): Path to PDF file
        
    Returns:
        dict: PDF information
    """
    try:
        reader = PdfReader(pdf_path)
        info = {
            'filename': os.path.basename(pdf_path),
            'total_pages': len(reader.pages),
            'size_mb': os.path.getsize(pdf_path) / (1024 * 1024),
            'needs_split': len(reader.pages) > 100
        }
        
        # Try to get metadata
        if reader.metadata:
            info['title'] = reader.metadata.get('/Title', 'Unknown')
            info['author'] = reader.metadata.get('/Author', 'Unknown')
            info['subject'] = reader.metadata.get('/Subject', 'Unknown')
        
        return info
        
    except Exception as e:
        return {
            'filename': os.path.basename(pdf_path),
            'error': str(e),
            'needs_split': False
        }


def process_directory(input_dir, max_pages=100):
    """
    Process all PDF files in a directory.
    
    Args:
        input_dir (str): Input directory path
        max_pages (int): Maximum pages per output file
    """
    input_path = Path(input_dir)
    
    if not input_path.exists():
        print(f"❌ Error: Directory {input_dir} does not exist")
        return
    
    # Create splits subdirectory
    splits_dir = input_path / "splits"
    
    # Find all PDF files
    pdf_files = list(input_path.glob("*.pdf"))
    
    if not pdf_files:
        print(f"📁 No PDF files found in {input_dir}")
        return
    
    print(f"🔍 Found {len(pdf_files)} PDF files")
    print("=" * 60)
    
    # Analyze all PDFs first
    pdf_info = []
    total_pages = 0
    files_to_split = 0
    
    for pdf_file in pdf_files:
        info = analyze_pdf(pdf_file)
        pdf_info.append(info)
        
        if 'total_pages' in info:
            total_pages += info['total_pages']
            if info['needs_split']:
                files_to_split += 1
    
    # Display analysis summary
    print("\n📊 PDF Analysis Summary:")
    print("-" * 40)
    for info in pdf_info:
        if 'error' in info:
            print(f"❌ {info['filename']}: Error - {info['error']}")
        else:
            status = "📄 LARGE" if info['needs_split'] else "✓ OK"
            size_str = f"{info['size_mb']:.1f}MB" if 'size_mb' in info else "Unknown size"
            print(f"{status} {info['filename']}: {info['total_pages']} pages ({size_str})")
    
    print(f"\n📈 Totals:")
    print(f"   • Total files: {len(pdf_files)}")
    print(f"   • Total pages: {total_pages}")
    print(f"   • Files needing split: {files_to_split}")
    print(f"   • Files under {max_pages} pages: {len(pdf_files) - files_to_split}")
    
    if files_to_split == 0:
        print("\n✅ All PDFs are already under the page limit!")
        return
    
    # Process files that need splitting
    print(f"\n🔧 Splitting {files_to_split} large PDF files...")
    print("=" * 60)
    
    total_split_files = 0
    
    for pdf_file in pdf_files:
        info = next(i for i in pdf_info if i['filename'] == pdf_file.name)
        
        if info.get('needs_split', False):
            split_files = split_pdf(str(pdf_file), str(splits_dir), max_pages)
            total_split_files += len(split_files)
    
    print("\n" + "=" * 60)
    print(f"✅ Processing Complete!")
    print(f"   • Created {total_split_files} split files in {splits_dir}")
    print(f"   • Each split file has ≤ {max_pages} pages")
    
    if total_split_files > 0:
        print(f"\n📁 Split files location: {splits_dir}")
        print("   You can now process these smaller files more easily.")


def main():
    """Main function."""
    parser = argparse.ArgumentParser(
        description="Split large PDF files into smaller chunks",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python pdf_splitter.py /path/to/pdfs/
  python pdf_splitter.py /path/to/pdfs/ --max-pages 50
  python pdf_splitter.py /path/to/pdfs/ --analyze-only
        """
    )
    
    parser.add_argument(
        'input_dir',
        help='Directory containing PDF files to split'
    )
    
    parser.add_argument(
        '--max-pages',
        type=int,
        default=100,
        help='Maximum pages per output file (default: 100)'
    )
    
    parser.add_argument(
        '--analyze-only',
        action='store_true',
        help='Only analyze PDFs without splitting them'
    )
    
    args = parser.parse_args()
    
    if args.max_pages < 1:
        print("❌ Error: max-pages must be at least 1")
        sys.exit(1)
    
    print("🔧 PDF Splitter Tool")
    print(f"📁 Input directory: {args.input_dir}")
    print(f"📄 Max pages per file: {args.max_pages}")
    print(f"🔍 Mode: {'Analysis only' if args.analyze_only else 'Split large files'}")
    print("=" * 60)
    
    if args.analyze_only:
        # Just analyze without splitting
        input_path = Path(args.input_dir)
        pdf_files = list(input_path.glob("*.pdf"))
        
        for pdf_file in pdf_files:
            info = analyze_pdf(pdf_file)
            if 'error' in info:
                print(f"❌ {info['filename']}: Error - {info['error']}")
            else:
                status = "NEEDS SPLIT" if info['needs_split'] else "OK"
                print(f"{status}: {info['filename']} ({info['total_pages']} pages)")
    else:
        # Full processing
        process_directory(args.input_dir, args.max_pages)


if __name__ == "__main__":
    main()