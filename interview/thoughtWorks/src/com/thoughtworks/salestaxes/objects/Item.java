package com.thoughtworks.salestaxes.objects;

import java.math.BigDecimal;

/**
 * ThoughtWorks - Sales Taxes
 * Item.java
 * Purpose: represent the item object
 * 
 * @author Yang Sun
 * @version 1.0 8/27/2013
 */
public class Item {

    public enum ItemType {
        BOOKS(true), 
        FOOD(true), 
        MEDICALPRODUCTS(true), 
        MISCELLANEOUS(false);

        private boolean isExemptible;

        private ItemType(boolean isExemptible) {
            this.isExemptible = isExemptible;
        }

        public boolean isExemptible() {
            return isExemptible;
        }

        public void setExemptible(boolean isExemptible) {
            this.isExemptible = isExemptible;
        }
    }

    private String name;
    private ItemType itemType;
    private BigDecimal price;
    private boolean isImported;

    public Item(String name, ItemType itemType, BigDecimal price, boolean isImported) {
        this.name = name;
        this.itemType = itemType;
        this.price = price;
        this.isImported = isImported;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public BigDecimal getPrice() {
        return price;
    }

    public void setPrice(BigDecimal price) {
        this.price = price;
    }

    public ItemType getItemType() {
        return itemType;
    }

    public void setItemType(ItemType itemType) {
        this.itemType = itemType;
    }

    public boolean isImported() {
        return isImported;
    }

    public void setImported(boolean isImported) {
        this.isImported = isImported;
    }

    @Override
    public String toString() {
        return this.name;
    }
}
