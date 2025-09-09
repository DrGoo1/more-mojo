# Email Notifications Setup for MoreMojo Builds

This document explains how to set up automatic email notifications for your GitHub Actions builds.

## Prerequisites

- A GitHub repository with the MoreMojo project
- Admin access to the repository to configure secrets
- An email account that can be used for sending notifications

## Setting Up GitHub Secrets

The email notification system requires three secrets to be configured in your GitHub repository:

1. **MAIL_USERNAME**: The email address used to send notifications
2. **MAIL_PASSWORD**: The password or app-specific password for the email account
3. **NOTIFICATION_EMAIL**: The email address where you want to receive notifications

### Steps to Add Secrets:

1. Go to your GitHub repository
2. Click on "Settings" tab
3. In the left sidebar, click on "Secrets and variables" → "Actions"
4. Click "New repository secret"
5. Add each of the three required secrets:

   - Name: `MAIL_USERNAME`  
     Value: `your-email@gmail.com`

   - Name: `MAIL_PASSWORD`  
     Value: `your-password-or-app-password`

   - Name: `NOTIFICATION_EMAIL`  
     Value: `where-to-receive@example.com`

> **Important Security Note:** 
> For Gmail and many other providers, you should use an app-specific password rather than your main account password. 
> See [Google's documentation on app passwords](https://support.google.com/accounts/answer/185833) for details.

## Using a Different Email Provider

The current configuration uses Gmail's SMTP server. If you prefer a different email provider:

1. Open `.github/workflows/build_app.yml` and `.github/workflows/build_plugin_mac.yml`
2. Locate the `Send email notification` step
3. Update the `server_address` and `server_port` values to match your provider's settings

Example for Office 365:
```yaml
server_address: smtp.office365.com
server_port: 587
```

## Testing the Notifications

To test if notifications are working:

1. Make a small change to the codebase
2. Commit and push to trigger the workflows
3. Check your email for notifications

If you don't receive an email, check the workflow run logs for any errors in the "Send email notification" step.

## Customizing Notification Content

You can customize the email subject and body by editing the workflow files:

1. Open the workflow files (`.github/workflows/build_*.yml`)
2. Find the `Send email notification` step
3. Modify the `subject` and `body` fields to include the information you need

## Troubleshooting

- **Not receiving emails**: Check spam folders and verify the SMTP settings
- **Authentication errors**: Make sure you're using the correct password or app-specific password
- **Workflow errors**: Check if the email sending action is failing in the Actions log
