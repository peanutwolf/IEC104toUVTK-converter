/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package uvtkreader;

import java.awt.Color;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DocumentFilter;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;


/**
 *
 * @author root
 */
public class DocumentSizeFilter extends DocumentFilter{

    private JTextArea area;
    private int max;
    private SimpleAttributeSet foreground;
    private JTextPane jTextPane1;

    public DocumentSizeFilter(JTextPane jTextPane1, JTextArea area, int max) {
        this.area = area;
        this.max = max;
        foreground = new SimpleAttributeSet();
        foreground.addAttribute(StyleConstants.Foreground, Color.RED);
        this.jTextPane1 = jTextPane1;
        //StyleConstants.setForeground(foreground, Color.red);
    }

    @Override
    public void insertString(FilterBypass fb, int offset,
            String text, AttributeSet attrs) throws BadLocationException {
       // System.out.println("Insert String" + offset);
        super.insertString(fb, offset, text, foreground); 
        
        int lines = area.getLineCount();
        if (lines > max) {
            int linesToRemove = lines - max -1;
            int lengthToRemove = area.getLineStartOffset(linesToRemove);           
            remove(fb, 0, lengthToRemove);
          //  System.out.println("Remove String" + lengthToRemove);
        }
    }
}
