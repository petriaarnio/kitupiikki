//
//  macanalyzerdocument.m
//  Kitsas
//
//  Created by Petri Aarnio on 29/05/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

#include <QMap>
#include "macanalyzerdocument.h"
#include "pdfanalyzerpage.h"

static CGPDFDocumentRef CreatePDFDocument(const UInt8 *data, const CFIndex length);

MacAnalyzerDocument::MacAnalyzerDocument(const QByteArray &data)
{
    pdfDoc_ = CreatePDFDocument((const UInt8 *)data.constData(), data.size());
    isUnlocked_ = CGPDFDocumentIsUnlocked(pdfDoc_);
}

MacAnalyzerDocument::~MacAnalyzerDocument()
{
    CGPDFDocumentRelease(pdfDoc_);
}

int MacAnalyzerDocument::pageCount()
{
    return (pdfDoc_ && isUnlocked_) ? (int)CGPDFDocumentGetNumberOfPages(pdfDoc_) : 0;
}

PdfAnalyzerPage MacAnalyzerDocument::page(int page)
{
    PdfAnalyzerPage result;
    if (pdfDoc_ && isUnlocked_) {
        
        CGPDFPageRef sivu = CGPDFDocumentGetPage(pdfDoc_, page);
        QMap<int, PdfAnalyzerRow> rows;
        
        if (sivu) {
            result.setSize( QSizeF::fromCGSize(CGPDFPageGetBoxRect(sivu, kCGPDFCropBox).size) );
            
            auto lista = sivu->textList();
            for(int i=0; i < lista.count(); i++) {
                auto ptr = lista.at(i);
                PdfAnalyzerText text;
                while(ptr) {
                    text.addWord( ptr->boundingBox(),
                                 ptr->text(),
                                 ptr->hasSpaceAfter());
                    
                    ptr = ptr->nextWord();
                    if( ptr )
                    i++;
                }
                int indeksi = qRound( text.boundingRect().top() );
                if( rows.contains(indeksi-1) )
                indeksi = indeksi -1;
                else if( rows.contains(indeksi+1))
                indeksi = indeksi + 1;
                
                // Jätetään pois sivumarginaalia
                if( text.boundingRect().right() > 25)
                rows[indeksi].addText(text);
            }
            delete sivu;
        }
        
        QMapIterator<int,PdfAnalyzerRow> iter(rows);
        while(iter.hasNext()) {
            iter.next();
            result.addRow(iter.value());
        }
    }
    
    return result;
}

QList<PdfAnalyzerPage> MacAnalyzerDocument::allPages()
{
    
}

QString MacAnalyzerDocument::title() const
{
    
}

static CGPDFDocumentRef CreatePDFDocument(const UInt8 *data, const CFIndex length)
{
    CFDataRef pdfData = CFDataCreateWithBytesNoCopy(NULL, data, length, NULL);
    if (pdfData == NULL)
    {
        fprintf(stderr, "CFData not created for PDF");
        return NULL;
    }
    
    CGDataProviderRef dataProvider = CGDataProviderCreateWithCFData(pdfData);
    CGPDFDocumentRef document = CGPDFDocumentCreateWithProvider(dataProvider);
    CGDataProviderRelease(dataProvider);
    CFRelease(data);
    
    size_t count = CGPDFDocumentGetNumberOfPages (document);
    if (count == 0) {
        printf("PDF needs at least one page!");
        return NULL;
    }
    
    return document;
}
