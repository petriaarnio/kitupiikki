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
#include <QImage>
#include <QSizeF>
#include <QList>

//extern void CreatePDFDocument(UInt8 *data, CFIndex length);
//extern bool PdfExists();
//extern void DeletePdfDocument();
//extern CGImageRef RenderPdfToImage(size_t pageNumber, int width, int height);

namespace Poppler {
    class TextBox
    {
        friend class Page;
        
    public:
        QString text() const;
        QRectF boundingBox() const;
        TextBox *nextWord() const;
    };

    class Page {
    public:
        enum Rotation
        {
            Rotate0 = 0, ///< Do not rotate
            Rotate90 = 1, ///< Rotate 90 degrees clockwise
            Rotate180 = 2, ///< Rotate 180 degrees
            Rotate270 = 3 ///< Rotate 270 degrees clockwise (90 degrees counterclockwise)
        };
        
        QSizeF pageSizeF() const;
        QImage renderToImage(double xres = 72.0, double yres = 72.0) const;
        QList<TextBox *> textList(Rotation rotate = Rotate0) const;
        QImage thumbnail() const;
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
        static Document *load(const QString &filePath, const QByteArray &ownerPassword = QByteArray(), const QByteArray &userPassword = QByteArray());
    };
}

#endif /* pdf_h */
