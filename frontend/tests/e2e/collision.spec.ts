import { test, expect } from '@playwright/test';

test.describe('Collision Display', () => {
    test('impact counter starts at zero', async ({ page }) => {
        await page.goto('/');
        // The impact counter should show 0 initially
        await expect(page.getByText('0')).toBeVisible();
    });

    test('impact counter element exists in HUD', async ({ page }) => {
        await page.goto('/');
        await expect(page.getByText('Impacts')).toBeVisible();
    });
});
