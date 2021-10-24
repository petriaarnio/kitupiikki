//
//  macanalyzerdocument.m
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
#import <Foundation/Foundation.h>
#include "macanalyzerdocument.h"
#include "pdfanalyzerpage.h"

#include <iostream>

#include <QMap>
#include <QSizeF>

MacAnalyzerDocument::MacAnalyzerDocument(const QByteArray &data)
{
    NSData *nsData = data.toNSData();
    pdfDoc_ = [[PDFDocument alloc] initWithData:nsData];
}

MacAnalyzerDocument::~MacAnalyzerDocument()
{
    if( pdfDoc_)
        [pdfDoc_ release];
}

int MacAnalyzerDocument::pageCount()
{
    if(pdfDoc_ && ![pdfDoc_ isLocked])
        return (int)[pdfDoc_ pageCount];
    else
        return 0;
}

PdfAnalyzerPage MacAnalyzerDocument::page(int page)
{
    PdfAnalyzerPage result;
    if( pdfDoc_ && ![pdfDoc_ isLocked]) {
        
        PDFPage *sivu = [pdfDoc_ pageAtIndex:page];
        QMap<int,PdfAnalyzerRow> rows;
        
        if( sivu) {
            CGSize cgSize = [sivu boundsForBox:kPDFDisplayBoxCropBox].size;
            QSizeF qSize = QSizeF::fromCGSize(cgSize);
            result.setSize(qSize);

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
            [sivu release];
        }
        
        QMapIterator<int,PdfAnalyzerRow> iter(rows);
        while(iter.hasNext()) {
            iter.next();
            result.addRow(iter.value());
            
            //            std::cerr << iter.key() << "   ";
            //            for(auto text: iter.value().textList() )
            //                std::cerr << text.text().toStdString() << " # ";
            //            std::cerr << "\n";
        }
    }
    
    return result;
}

QList<PdfAnalyzerPage> MacAnalyzerDocument::allPages()
{
    QList<PdfAnalyzerPage> pages;
    for(int i=0; i < pageCount(); i++)
        pages.append( page(i) );
    return pages;
}

QString MacAnalyzerDocument::title() const
{
    if( pdfDoc_)
        return pdfDoc_->title();
    else
        return QString();
}
