//
//  pdf.m
//  Kitsas
//
//  Created by Petri Aarnio on 21/03/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "pdf.h"

static void DrawPdfPage (CGContextRef context, CGPDFDocumentRef document, size_t pageNumber);
static CGContextRef CreateBitmapContext(int pixelsWide, int pixelsHigh);

static CGPDFDocumentRef pdfDocument = NULL;

void CreatePDFDocument(UInt8 *data, CFIndex length)
{
    CFDataRef pdfData = CFDataCreateWithBytesNoCopy(NULL, data, length, NULL);
    if (pdfData == NULL)
    {
        fprintf(stderr, "CFData not created for PDF");
        return;
    }

    CGDataProviderRef dataProvider = CGDataProviderCreateWithCFData(pdfData);
    CGPDFDocumentRef document = CGPDFDocumentCreateWithProvider(dataProvider);
    CGDataProviderRelease(dataProvider);
    CFRelease(data);
    
    size_t count = CGPDFDocumentGetNumberOfPages (document);
    if (count == 0) {
        printf("PDF needs at least one page!");
        return;
    }

    pdfDocument = document;
}

bool PdfExists()
{
    return pdfDocument != NULL;
}


size_t NumPages()
{
    return pdfDocument ? CGPDFDocumentGetNumberOfPages(pdfDocument) : 0;
}

void DeletePdfDocument() {
    CGPDFDocumentRelease (pdfDocument);
    pdfDocument = NULL;
}

CGImageRef RenderPdfToImage(size_t pageNumber, int width, int height)
{
    CGContextRef bitmapContext = CreateBitmapContext(width, height);
    DrawPdfPage(bitmapContext, pdfDocument, pageNumber);
    CGImageRef image = CGBitmapContextCreateImage(bitmapContext);
    return image;
}

static void DrawPdfPage (CGContextRef context, CGPDFDocumentRef document, size_t pageNumber)
{
    CGPDFPageRef page = CGPDFDocumentGetPage(document, pageNumber);
    CGContextDrawPDFPage(context, page);
}

static CGContextRef CreateBitmapContext(int pixelsWide, int pixelsHigh)
{
    CGContextRef context = NULL;
    CGColorSpaceRef colorSpace;
    void *bitmapData;
    int bitmapByteCount;
    int bitmapBytesPerRow;
    
    bitmapBytesPerRow = (pixelsWide * 4);
    bitmapByteCount = (bitmapBytesPerRow * pixelsHigh);
    
    colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    bitmapData = calloc( bitmapByteCount, sizeof(uint8_t) );
    if (bitmapData == NULL)
    {
        fprintf (stderr, "Memory not allocated!");
        return NULL;
    }
    context = CGBitmapContextCreate(bitmapData,
                                    pixelsWide,
                                    pixelsHigh,
                                    8,      // bits per component
                                    bitmapBytesPerRow,
                                    colorSpace,
                                    kCGImageAlphaPremultipliedLast);
    if (context == NULL)
    {
        free (bitmapData);
        fprintf (stderr, "Context not created!");
        return NULL;
    }
    CGColorSpaceRelease( colorSpace );
    
    return context;
}

namespace Poppler {
    Document *Document::loadFromData(const QByteArray &fileContents)
    {
    }
    
    void Document::setRenderHint(RenderHint hint)
    {
    
    }

    int Document::numPages() const {
        
    }
    
    Page *Document::page(int index) const
    {
        
    }
    
    QString Document::info(const QString &type) const
    {
        
    }
}
