/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package uvtkreader;

import java.awt.Color;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultStyledDocument;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyleContext;

/**
 *
 * @author root
 */
public class DefaultSizeStyledDocument extends DefaultStyledDocument{
    final StyleContext cont;
    final AttributeSet attrGray, attrBlack, attrRed_FE, attrGreen_KZP;
    Pattern timePattern, KZPpattern, FEpattern;
    Matcher timeMatcher, KZPmatcher, FEmatcher;
    private int lineCount;
    private int maxLength;
    public DefaultSizeStyledDocument(int maxLength){
       this.lineCount = 1;
       this.maxLength = maxLength;
       cont = StyleContext.getDefaultStyleContext();
       this.attrGray = cont.addAttribute(cont.getEmptySet(), StyleConstants.Foreground, Color.GRAY);
       this.attrRed_FE = cont.addAttribute(cont.getEmptySet(), StyleConstants.Background, Color.RED);
       this.attrGreen_KZP = cont.addAttribute(cont.getEmptySet(), StyleConstants.Background, Color.GREEN);
       attrBlack = cont.addAttribute(cont.getEmptySet(), StyleConstants.Foreground, Color.BLACK);
       this.timePattern = Pattern.compile("\\d{2}:\\d{2}:\\d{2}");  
       this.KZPpattern = Pattern.compile("0xFE 11111110");
       this.FEpattern = Pattern.compile("0x7E 01111110");
    }
    
    @Override
    public void insertString (int offset, String str, AttributeSet a) throws BadLocationException {
        String tmpByteString = "";
      this.timeMatcher = timePattern.matcher(str);
      this.KZPmatcher = KZPpattern.matcher(str);
      this.FEmatcher = FEpattern.matcher(str);
        if(timeMatcher.find()){
            super.insertString(this.getLength(), str.substring(timeMatcher.start(), (timeMatcher.end())), attrGray);
            tmpByteString = str.substring(timeMatcher.end());
            if(KZPmatcher.find()){
                super.insertString(this.getLength(), str.substring(KZPmatcher.start()), attrGreen_KZP);
            }
            else if(FEmatcher.find()){
                super.insertString(this.getLength(), str.substring(FEmatcher.start()), attrRed_FE);
            }
            else
                super.insertString(this.getLength(), tmpByteString, attrBlack); 
        }
        else if(KZPmatcher.find()){
            super.insertString(this.getLength(), str.substring(0, KZPmatcher.start()), attrBlack);
            super.insertString(this.getLength(), str.substring(KZPmatcher.start()), attrGreen_KZP);
        }
        else if(FEmatcher.find()){
            super.insertString(this.getLength(), str.substring(0, FEmatcher.start()), attrBlack);
            super.insertString(this.getLength(), str.substring(FEmatcher.start()), attrRed_FE);
        }
        else
            super.insertString(this.getLength(), str, attrBlack); 
        
        if (this.getText(0, getLength()).split("\n").length >= maxLength){
            
           remove(0, getText(0, getLength()).indexOf("\n")+1);
        }    
        
     }  
    @Override
    public void remove(int offs, int len){
        try {
            super.remove(offs, len);
         //   System.out.println("Remove String");
        } catch (BadLocationException ex) {
            Logger.getLogger(DefaultSizeStyledDocument.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
  
}
