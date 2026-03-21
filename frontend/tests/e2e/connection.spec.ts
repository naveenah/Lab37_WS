import { test, expect } from '@playwright/test';

test.describe('Connection State', () => {
    test('shows connection status indicator', async ({ page }) => {
        await page.goto('/');
        // Without backend, should show Connecting or Disconnected
        await expect(
            page.getByText(/Connecting|Disconnected|Reconnecting/),
        ).toBeVisible();
    });

    test('displays FPS and latency metrics', async ({ page }) => {
        await page.goto('/');
        await expect(page.getByText(/FPS:/)).toBeVisible();
        await expect(page.getByText(/Latency:/)).toBeVisible();
    });

    test('shows input source indicator', async ({ page }) => {
        await page.goto('/');
        await expect(page.getByText('INPUT')).toBeVisible();
    });

    test('app renders without crashing', async ({ page }) => {
        const errors: string[] = [];
        page.on('pageerror', (err) => errors.push(err.message));

        await page.goto('/');
        await page.waitForTimeout(1000);

        // No JS errors should have occurred
        expect(errors).toHaveLength(0);
    });
});
