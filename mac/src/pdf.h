//
//  pdf.h
//  Kitsas
//
//  Created by Petri Aarnio on 21/03/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

#ifndef pdf_h
#define pdf_h

extern void CreatePDFDocument(UInt8 *data, CFIndex length);
extern void DeletePdfDocument();
extern CGImageRef RenderPdfToImage(size_t pageNumber, int width, int height);

#endif /* pdf_h */
