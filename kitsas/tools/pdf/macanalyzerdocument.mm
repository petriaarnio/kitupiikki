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
        
        if (sivu) {
            CGSize cgSize = [sivu boundsForBox:kPDFDisplayBoxCropBox].size;
            NSLog(@"Width: %f", cgSize.width);
            NSLog(@"Height: %f", cgSize.height);

            QSizeF qSize = QSizeF::fromCGSize(cgSize);
            NSLog(@"QWidth: %f", qSize.width());
            NSLog(@"QHeight: %f", qSize.height());

            result.setSize(qSize);
            NSString *pageText = [sivu string];
            //NSArray *words = [text componentsSeparatedByString:@" "];
            NSUInteger pageTextLen = [pageText length];
            for (NSUInteger i = 0; i < pageTextLen; i++) {
                CGRect firstCharacterBounds = [sivu characterBoundsAtIndex:i];
                firstCharacterBounds.origin.y = cgSize.height - firstCharacterBounds.origin.y - firstCharacterBounds.size.height;
                CGRect wordBounds;
                wordBounds.origin = firstCharacterBounds.origin;
                wordBounds.size.height = firstCharacterBounds.size.height;
//                NSLog(@"EKA: %f", firstCharacterBounds.size.width);

                PdfAnalyzerText text;
                NSMutableString *word = [NSMutableString string];
                NSUInteger wordWidth = 0;
                for (NSUInteger j = 0; j < pageTextLen; j++) {
                    unichar character = [pageText characterAtIndex:(i + j)];
                    if (character == ' ' || character == '\n') {
                        i += j;
                        break;
                    }
                    [word appendFormat:@"%C", character];
                    CGRect characterBounds = [sivu characterBoundsAtIndex:(i + j)];
                    wordWidth += characterBounds.size.width;
                }
                if (![word length]) { continue; }
                wordBounds.size.width = wordWidth;
                QRectF qWordBounds = QRectF::fromCGRect(wordBounds);
                QString qWord = QString::fromNSString(word);
                text.addWord(qWordBounds, qWord, false);
//                CGRect bounds = [sivu characterBoundsAtIndex:i];
//                NSLog(@"%hu", [pageText characterAtIndex:i]);
                NSLog(@"%@", word);
                NSLog(@"-----------");
                NSLog(@"X: %f", qWordBounds.x());
                NSLog(@"Y: %f", qWordBounds.y());
                NSLog(@"Length: %f", qWordBounds.width());
                NSLog(@"Height: %f", qWordBounds.height());
                NSLog(@"");
//                CGRect bounds:
//                bounds.origin = [word characterBoundsAtIndex:0];

//                text.addWord(QRectF::fromCGRect(bounds),
//                             ,
//                             ptr->hasSpaceAfter());

                int indeksi = qRound( text.boundingRect().top() );
                if( rows.contains(indeksi - 1) )
                    indeksi--;
                else if( rows.contains(indeksi + 1))
                    indeksi++;

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
            
                        std::cerr << iter.key() << "   ";
                        for(auto text: iter.value().textList() )
                            std::cerr << text.text().toStdString() << " # ";
                        std::cerr << "\n";
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
        return QString::fromNSString([pdfDoc_ documentAttributes][PDFDocumentTitleAttribute]);
    else
        return QString();
}
