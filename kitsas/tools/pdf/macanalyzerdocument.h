//
//  macanalyzerdocument.h
//  Kitsas
//
//  Created by Petri Aarnio on 29/05/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

#ifndef macanalyzerdocument_h
#define macanalyzerdocument_h

#import <Foundation/Foundation.h>
#include "pdftoolkit.h"

class MacAnalyzerDocument : public PdfAnalyzerDocument
{
public:
    MacAnalyzerDocument(const QByteArray& data);
    ~MacAnalyzerDocument();
    
    virtual int pageCount() override;
    virtual PdfAnalyzerPage page(int page) override;
    virtual QList<PdfAnalyzerPage> allPages() override;
    virtual QString title() const override;
    
    
private:
    CGPDFDocumentRef pdfDoc_ = NULL;
    bool isUnlocked_ = false;
};

#endif /* macanalyzerdocument_h */
