package com.thoughtworks.salestaxes.io;

import java.io.FileNotFoundException;
import java.math.BigDecimal;

import com.thoughtworks.salestaxes.checkout.ItemEntry;
import com.thoughtworks.salestaxes.checkout.Receipt;
import com.thoughtworks.salestaxes.objects.Item;
import com.thoughtworks.salestaxes.objects.Item.ItemType;

/**
 * ThoughtWorks - Sales Taxes
 * OrderProcessor.java
 * Purpose: drive the task
 * 
 * @author Yang Sun
 * @version 1.0 8/27/2013
 */
public class OrderProcessor {

    public static void main(String[] args) throws FileNotFoundException {
        Receipt receipt1 = new Receipt();
        Item book1 = new Item("book", ItemType.BOOKS, new BigDecimal(12.49), false);
        Item musicCD1 = new Item("music CD", ItemType.MISCELLANEOUS, new BigDecimal(14.99), false);
        Item chocolateBar1 = new Item("chocolate bar", ItemType.FOOD, new BigDecimal(0.85), false);
        receipt1.addItemEntryWithTax(new ItemEntry(book1, 1));
        receipt1.addItemEntryWithTax(new ItemEntry(musicCD1, 1));
        receipt1.addItemEntryWithTax(new ItemEntry(chocolateBar1, 1));
        System.out.println(receipt1);

        Receipt receipt2 = new Receipt();
        Item importedChocolate2 = new Item("imported box of chocolates", ItemType.FOOD, new BigDecimal(10.00), true);
        Item importedPerfume2 = new Item("imported bottle of perfume", ItemType.MISCELLANEOUS, new BigDecimal(47.50),
                true);
        receipt2.addItemEntryWithTax(new ItemEntry(importedChocolate2, 1));
        receipt2.addItemEntryWithTax(new ItemEntry(importedPerfume2, 1));
        System.out.println(receipt2);

        Receipt receipt3 = new Receipt();
        Item importedPerfume3 = new Item("imported bottle of perfume", ItemType.MISCELLANEOUS, new BigDecimal(27.99),
                true);
        Item perfume3 = new Item("bottle of perfume", ItemType.MISCELLANEOUS, new BigDecimal(18.99), false);
        Item headachePills3 = new Item("packet of headache pills", ItemType.MEDICALPRODUCTS, new BigDecimal(9.75),
                false);
        Item importedChocolate = new Item("box of imported chocolates", ItemType.FOOD, new BigDecimal(11.25), true);
        receipt3.addItemEntryWithTax(new ItemEntry(importedPerfume3, 1));
        receipt3.addItemEntryWithTax(new ItemEntry(perfume3, 1));
        receipt3.addItemEntryWithTax(new ItemEntry(headachePills3, 1));
        receipt3.addItemEntryWithTax(new ItemEntry(importedChocolate, 1));
        System.out.println(receipt3);

    }
}
