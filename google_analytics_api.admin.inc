<?php
// $Id: google_analytics_api.admin.inc,v 1.12 2010/01/09 01:43:55 jkitching Exp $

/**
 * @file
 * Provides the interface whereby one can grant access to a Google account and
 * select the appropriate Analytics account.
 */

/**
 * The admin form.
 */
function google_analytics_api_admin() {
	$form = array();
	$gapi = google_analytics_api_gapi();
	
	if ($gapi) {
		$form['login_status'] = array(
	      '#value' => '<p>' . t('Currently you have granted access to a Google Analytics account. Please choose the profile below that contains statistics for this site.') . '</p>',
	      '#weight' => -10,
	    );
	}
	else {
		$form['logout_status'] = array(
	      '#value' => '<p>' . t('In order for you to access your Google Analytics account through Drupal, you need to grant access to your account. This gives Drupal permission to contact the Google Analytics server and download statistics about your site.') . '</p>',
	      '#weight' => -10,
	    );
	}
	
	$form['client_id'] = array(
		'#type' => 'textfield',
		'#title' => t('Client ID'),
		'#default_value' => variable_get('google_analytics_api_client_id', ''),
		'#size' => 30,
		'#description' => t('Client ID created for the app in the access tab of the ') . l('Google API Console', 'http://code.google.com/apis/console', array('attributes' => array('target' => '_blank'))),
		'#weight' => -9,
	);

	$form['client_secret'] = array(
		'#type' => 'textfield',
		'#title' => t('Client Secret'),
		'#default_value' => variable_get('google_analytics_api_client_secret', ''),
		'#size' => 30,
		'#description' => t('Client Secret created for the app in the Google API Console'),
		'#weight' => -8,
	);
	
	if ($gapi) {
    	$form['profile_id'] = array(
	      '#type' => 'select',
	      '#title' => t('Profile'),
	      '#options' => google_analytics_api_admin_account_data_options(),
	      '#default_value' => variable_get('google_analytics_api_profile_id', 0),
	      '#description' => t("Choose your website's profile."),
	      '#required' => TRUE,
	      '#weight' => -2,
	    );
	    $form['logout'] = array(
	      '#type' => 'submit',
	      '#value' => t('Revoke access'),
	      '#weight' => 0,
	    );
	    $form['save'] = array(
	      '#type' => 'submit',
	      '#value' => t('Save configuration'),
	      '#weight' => 3,
	    );
	  }
  else {
    $form['auth'] = array(
      '#type' => 'submit',
      '#value' => t('Grant access to a Google account'),
    );
  }

  return $form;
}

/*
* Validate the configuration form. If Client ID or Client Secret
* are not filled in, throw an error.
*/
function google_analytics_api_admin_validate($form, &$form_state) {
	if (!$form_state['values']['client_id'] || !$form_state['values']['client_secret']) {
		form_set_error('', t('You must enter a valid client ID and client Secret.'));
	}
}

/*
 * Returns an associative array of profile IDs and their names for an account.
 */
function google_analytics_api_admin_account_data_options() {
  $accounts = google_analytics_api_account_data();
  $options = array();
  $i = 0;
  if (!$accounts) {
    return array(variable_get('google_analytics_api_profile_id', 0) => t('Account listing unavailable. Currently selected account has ID: @profile-id.', array('@profile-id' => variable_get('google_analytics_api_profile_id', 0))));
  }
  foreach ($accounts as $account) {
    $options[$account->getProfileId()] = t('@title (@profile-id)', array('@title' => $account->getTitle(), '@profile-id' => $account->getProfileId()));
  }
  return $options;
}

/**
 * Submit handler. Begins the OAuth process, revokes tokens, saves profiles.
*/
function google_analytics_api_admin_submit($form, &$form_state) {
	$op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
	cache_clear_all('bw:ga_reports_data', 'cache', true);
	cache_clear_all('bw:ga_reports_totals', 'cache', true);
	switch ($op) {
	  case t('Grant access to a Google account'):
			variable_set('google_analytics_api_client_id', $form_state['values']['client_id']);
  		variable_set('google_analytics_api_client_secret', $form_state['values']['client_secret']);
  		google_analytics_api_authenticate($form_state['values']['client_id']);
  		break;
		case t('Save configuration'):
	    variable_set('google_analytics_api_profile_id', intval($form_state['values']['profile_id']));
	    drupal_set_message(t('Settings have been saved successfully.'));
	    break;
	  case t('Revoke access'):
	    drupal_set_message(t('You have revoked access to your Google account.'));
		  google_analytics_api_logout();
	    break;
	}
}