package com.thoughtworks.salestaxes.checkout;

import java.math.BigDecimal;

import com.thoughtworks.salestaxes.objects.Item;
import com.thoughtworks.salestaxes.tax.TaxCalculator;

/**
 * ThoughtWorks - Sales Taxes
 * ItemEntry.java
 * Purpose: represent the itemEntry that is shown on the receipt
 * 
 * @author Yang Sun
 * @version 1.0 8/27/2013
 */
public class ItemEntry {
    private Item item;
    private int quantity;

    public ItemEntry(Item item, int quantity) {
        this.item = item;
        this.quantity = quantity;
    }

    /**
     * Calculate the tax and update the total price
     * @return the new total price including tax
     */
    public BigDecimal getTotalPriceAfterTax() {
        return getTotalPriceBeforeTax().add(TaxCalculator.apply(this));
    }

    /**
     * Calculate the total price in terms of quantity
     * @return the new total price excluding tax
     */
    public BigDecimal getTotalPriceBeforeTax() {
        return item.getPrice().multiply(new BigDecimal(quantity));
    }

    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }

    public int getQuantity() {
        return quantity;
    }

    public void setQuantity(int quantity) {
        this.quantity = quantity;
    }

    @Override
    public String toString() {
        return quantity + (item.isImported() ? " imported " : " ") + item.getName();
    }
}
