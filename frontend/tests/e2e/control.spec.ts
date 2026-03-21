import { test, expect } from '@playwright/test';

test.describe('Robot Control', () => {
    test('page loads with simulation canvas', async ({ page }) => {
        await page.goto('/');
        // Canvas should be present
        const canvas = page.locator('canvas');
        await expect(canvas).toBeVisible();
    });

    test('HUD elements are visible', async ({ page }) => {
        await page.goto('/');
        // Connection status should be visible
        await expect(page.getByText(/Connecting|Connected|Disconnected|Streaming/)).toBeVisible();
        // Impact counter label
        await expect(page.getByText('Impacts')).toBeVisible();
        // Control hints
        await expect(page.getByText('W/S - Throttle')).toBeVisible();
    });

    test('throttle gauge shows on page', async ({ page }) => {
        await page.goto('/');
        await expect(page.getByText('THROTTLE')).toBeVisible();
        await expect(page.getByText('STEERING')).toBeVisible();
    });

    test('keyboard input does not cause errors', async ({ page }) => {
        await page.goto('/');
        // Ensure no console errors from key input
        const errors: string[] = [];
        page.on('pageerror', (err) => errors.push(err.message));

        await page.keyboard.press('KeyW');
        await page.waitForTimeout(200);
        await page.keyboard.press('KeyA');
        await page.waitForTimeout(200);
        await page.keyboard.press('Space');
        await page.waitForTimeout(200);

        expect(errors).toHaveLength(0);
    });
});
