package com.thoughtworks.salestaxes.checkout;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.LinkedList;
import java.util.ListIterator;

import com.thoughtworks.salestaxes.tax.TaxCalculator;

/**
 * ThoughtWorks - Sales Taxes
 * Receipt.java
 * Purpose: represent the receipt object, which contains itemEntries
 * 
 * @author Yang Sun
 * @version 1.0 8/27/2013
 */
public class Receipt {

    private LinkedList<ItemEntry> itemEntries;
    private BigDecimal totalPrice;
    private BigDecimal salesTaxes;

    public Receipt() {
        // Since the receipt can be super long, LinkedList is more appropriate than ArrayList
        itemEntries = new LinkedList<ItemEntry>();
        
        // Initially, totalPrice and salesTaxes are 0.0
        totalPrice = new BigDecimal(0.0);
        salesTaxes = new BigDecimal(0.0);
    }

    /**
     * Add a new itemEntry to the receipt and update the total price and sales taxes
     * @param itemEntry the target itemEntry needs to be added to the receipt
     * @return true
     */
    public boolean addItemEntryWithTax(ItemEntry itemEntry) {
        itemEntries.add(itemEntry);
        salesTaxes = salesTaxes.add(TaxCalculator.apply(itemEntry));
        totalPrice = totalPrice.add(itemEntry.getTotalPriceAfterTax());
        return true;
    }

    public LinkedList<ItemEntry> getItemEntries() {
        return itemEntries;
    }

    public void setItemEntries(LinkedList<ItemEntry> itemEntries) {
        this.itemEntries = itemEntries;
    }

    public BigDecimal getTotalPrice() {
        return totalPrice;
    }

    public void setTotalPrice(BigDecimal totalPrice) {
        this.totalPrice = totalPrice;
    }

    public BigDecimal getSalesTaxes() {
        return salesTaxes;
    }

    public void setSalesTaxes(BigDecimal salesTaxes) {
        this.salesTaxes = salesTaxes;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        ListIterator<ItemEntry> itemEntriesItor = itemEntries.listIterator();
        while (itemEntriesItor.hasNext()) {
            ItemEntry itemEntry = itemEntriesItor.next();
            sb.append(itemEntry.getQuantity() + " " + itemEntry.getItem().getName() + ": "
                    + itemEntry.getTotalPriceAfterTax().setScale(2, RoundingMode.HALF_UP) + "\n");
        }
        sb.append("Sales Taxes: " + getSalesTaxes().setScale(2, RoundingMode.HALF_UP) + "\n");
        sb.append("Total: " + getTotalPrice().setScale(2, RoundingMode.HALF_UP) + "\n");
        return sb.toString();
    }

}
