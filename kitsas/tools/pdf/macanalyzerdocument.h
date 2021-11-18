//
//  macanalyzerdocument.h
//  Kitsas
//
//  Created by Petri Aarnio on 29/05/2021.
//  Copyright © 2021 Atfos Oy. All rights reserved.
//

/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MACANALYZERDOCUMENT_H
#define MACANALYZERDOCUMENT_H

#import <Quartz/Quartz.h>
#include <poppler/qt5/poppler-qt5.h>
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
//    QList<<#typename T#>> textList(PDFPage *sivu) const;
    PDFDocument *pdfDoc_ = nullptr;
};

#endif // MACANALYZERDOCUMENT_H
