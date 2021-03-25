//
//  pdf.h
//  Kitsas
//
//  Created by Petri Aarnio on 21/03/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

#ifndef pdf_h
#define pdf_h
#include <QByteArray>
#include <QString>

//extern void CreatePDFDocument(UInt8 *data, CFIndex length);
//extern bool PdfExists();
//extern void DeletePdfDocument();
//extern CGImageRef RenderPdfToImage(size_t pageNumber, int width, int height);

namespace Poppler {
    class Page {
    public:
        QSizeF pageSizeF() const;
        QImage renderToImage(double xres = 72.0, double yres = 72.0) const;
    };

    class Document {
    public:
        enum RenderHint
        {
            Antialiasing = 0x00000001, ///< Antialiasing for graphics
            TextAntialiasing = 0x00000002, ///< Antialiasing for text
            TextHinting = 0x00000004, ///< Hinting for text \since 0.12.1
            TextSlightHinting = 0x00000008, ///< Lighter hinting for text when combined with TextHinting \since 0.18
            OverprintPreview = 0x00000010, ///< Overprint preview \since 0.22
            ThinLineSolid = 0x00000020, ///< Enhance thin lines solid \since 0.24
            ThinLineShape = 0x00000040, ///< Enhance thin lines shape. Wins over ThinLineSolid \since 0.24
            IgnorePaperColor = 0x00000080, ///< Do not compose with the paper color \since 0.35
            HideAnnotations = 0x00000100 ///< Do not render annotations \since 0.60
        };
        
        static Document *loadFromData(const QByteArray &fileContents);
        void setRenderHint(RenderHint hint);
        int numPages() const;
        Page *page(int index) const;
        QString info(const QString &type) const;
    };
}

#endif /* pdf_h */
