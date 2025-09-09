# Setting Up Slack Notifications for GitHub Actions

This guide explains how to set up Slack notifications for your GitHub Actions workflows.

## Step 1: Create a Slack App and Webhook

1. Go to [Slack API Apps page](https://api.slack.com/apps)
2. Click "Create New App"
3. Select "From scratch"
4. Give your app a name (e.g., "MoreMojo Build Bot") and select your workspace
5. Click "Create App"

### Enable Incoming Webhooks

1. In your app's settings, click on "Incoming Webhooks"
2. Toggle "Activate Incoming Webhooks" to On
3. Click "Add New Webhook to Workspace"
4. Select the channel where notifications should appear
5. Click "Allow"
6. Copy the Webhook URL that appears (it will look like `https://hooks.slack.com/services/T00000000/B00000000/XXXXXXXXXXXXXXXXXXXXXXXX`)

## Step 2: Add Webhook URL to GitHub Secrets

1. Go to your GitHub repository: https://github.com/DrGoo1/more-mojo
2. Click on "Settings" tab
3. In the left sidebar, click "Secrets and variables" → "Actions"
4. Click "New repository secret"
5. Name: `SLACK_WEBHOOK_URL`
6. Value: Paste the webhook URL you copied from Slack
7. Click "Add secret"

## Step 3: Customize Notifications (Optional)

If you want to customize the Slack notifications, you can edit the workflow files:

```yaml
- name: Send Slack notification
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    fields: repo,message,commit,author,action,eventName,workflow
    # Customize the message format:
    text: "Custom notification message here"
  env:
    SLACK_WEBHOOK_URL: ${{ secrets.SLACK_WEBHOOK_URL }}
```

Available status options:
- `success` (green)
- `failure` (red)
- `cancelled` (gray)
- `${{ job.status }}` (uses the actual job status)

## Step 4: Test the Notification

1. Go to GitHub Actions: https://github.com/DrGoo1/more-mojo/actions
2. Run the test workflow manually
3. Check your Slack channel for the notification

## Troubleshooting

- **No notifications received**: Verify the webhook URL is correct in GitHub secrets
- **Permission errors**: Make sure the Slack App has the proper permissions in your workspace
- **Invalid payload errors**: Check the message format in the workflow file
