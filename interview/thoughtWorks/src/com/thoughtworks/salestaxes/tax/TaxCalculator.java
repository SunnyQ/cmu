package com.thoughtworks.salestaxes.tax;

import java.math.BigDecimal;

import com.thoughtworks.salestaxes.checkout.ItemEntry;

/**
 * ThoughtWorks - Sales Taxes
 * TaxCalculator.java
 * Purpose: used to calculate sales taxes for items
 * 
 * @author Yang Sun
 * @version 1.0 8/27/2013
 */
public class TaxCalculator {

    /* the factor used to round up the tax */
    private static final BigDecimal ROUNDFACTOR = new BigDecimal(0.05);

    private enum TaxType {
        // basic sales tax: 10%
        BASIC(new BigDecimal(0.1)),
        
        // duty sales tax: 5%
        DUTY(new BigDecimal(0.05));

        private BigDecimal taxRate;

        private TaxType(BigDecimal taxRate) {
            this.taxRate = taxRate;
        }

        public BigDecimal getTaxRate() {
            return taxRate;
        }
    }

    /**
     * Apply the sales taxes to itemEntry. 
     * Depending on the item category, use the proper tax method. 
     * @param itemEntry the itemEntry that the tax will be applied to
     * @return the total amount of tax for this itemEntry
     */
    public static BigDecimal apply(ItemEntry itemEntry) {
        BigDecimal tax = new BigDecimal(0.0);
        
        // apply the basic sales tax if the item is not exemptible
        if (!itemEntry.getItem().getItemType().isExemptible()) {
            tax = tax.add(round(itemEntry.getTotalPriceBeforeTax().multiply(TaxType.BASIC.getTaxRate())));
        }

        // apply the duty tax if the item is imported
        if (itemEntry.getItem().isImported()) {
            tax = tax.add(round(itemEntry.getTotalPriceBeforeTax().multiply(TaxType.DUTY.getTaxRate())));
        }

        return tax;
    }

    /**
     * Round up the tax to the nearest 0.05 (ROUNDFACTOR)
     * @param tax the original tax amount
     * @return the new approximate rounded tax amount
     */
    private static BigDecimal round(BigDecimal tax) {
        tax = tax.divide(ROUNDFACTOR);
        tax = new BigDecimal(Math.ceil(tax.doubleValue()));
        tax = tax.multiply(ROUNDFACTOR);
        return tax;
    }
}
