<?php

/**
 * @file
 * Admin page callbacks for the page_theme module.
 */


/**
 * Menu callback; lists all defined rules.
 */
function page_theme_admin_overview($form, &$form_state) {

  $rules = array();

  foreach (page_theme_get_rules() as $rule) {

    $row['name'] = array(
      '#markup' => check_plain($rule['name']) . '<br><small>' . t('(Machine name: @rule)', array('@rule' => $rule['rule'])) . '</small>',
    );
    $row['theme'] = array(
      '#markup' => page_theme_get_theme_name($rule['theme'], TRUE),
    );
    $row['pages'] = array(
      '#markup' => nl2br(check_plain($rule['pages'])),
    );
    $row['roles'] = array(
      '#markup' => theme('item_list', array('items' => array_map('check_plain', page_theme_get_rule_roles($rule)))),
    );
    $row['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => $rule['status'],
    );
    $row['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $rule['weight'],
      '#delta' => 50,
    );
    $row['operations'] = array(
      '#type' => 'operations',
      '#links' => array(
        'configure' => array(
          'title' => t('Configure'),
          'href' => 'admin/appearance/page-theme/manage/'. strtr($rule['rule'], array('_' => '-')) .'/configure',
        ),
        'delete' => array(
          'title' => t('Delete'),
          'href' => 'admin/appearance/page-theme/manage/'. strtr($rule['rule'], array('_' => '-')) .'/delete',
        ),
      ),
    );

    $rules[$rule['rule']] = $row;
  }

  $form['information'] = array(
    array('#markup' => '<p>' . t('If pages are several defined, the first rule in the list will be used.') . '</p>'),
    array('#markup' => '<p>' . t('Only themes, which are enabled in the <a href="@themes_section">themes section</a>, will be used otherwise the site default theme.', array('@themes_section' => url('admin/appearance'))) . '</p>'),
  );
  $form['rules'] = array('#tree' => TRUE);
  $form['rules'] += $rules;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));

  return $form;
}

function page_theme_admin_overview_submit($form, &$form_state) {

  foreach ($form_state['values']['rules'] as $rule => $data) {

    config('page_theme.rule.' . $rule)
      ->set('status', $data['status'])
      ->set('weight', $data['weight'])
      ->save();
  }

  backdrop_static_reset('page_theme_get_rules_enabled');

  backdrop_set_message(t('The configuration has been saved.'));
}


/**
 * Menu callback; adds a rule.
 */
function page_theme_admin_add($form, &$form_state) {

  backdrop_set_title(t('Add rule'));

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('The human-readable name of this rule. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique.'),
    '#default_value' => '',
    '#required' => TRUE,
    '#size' => 30,
  );
  $form['rule'] = array(
    '#type' => 'machine_name',
    '#description' => t('A unique machine-readable name for this rule. It must only contain lowercase letters, numbers, and underscores. This name will be used for constructing the URL of the %page-theme page, in which underscores will be converted into hyphens.', array('%page-theme' => t('Page theme'))),
    '#default_value' => '',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'page_theme_rule_load',
    ),
  );
  $form['theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme'),
    '#description' => t('Choose which theme the page(s) should display in.'),
    '#default_value' => '0',
    '#options' => page_theme_get_themes_options(),
    '#required' => TRUE,
  );
  $form['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#description' => t('Enter one path per line. The "*" character is a wildcard. Example paths are "node/1" for an individual piece of content or "node/*" for every piece of content. "@front" is the front page.', array('@front' => '<front>')),
    '#default_value' => '',
    '#required' => TRUE,
  );
  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles'),
    '#description' => t('Show this theme only for the selected role(s). If you select no roles, the theme will be visible to all users.'),
    '#default_value' => array(),
    '#options' => array_map('check_plain', user_roles()),
  );
  $form['status'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Status'),
    '#default_value' => array('enabled'),
    '#options' => array(
      'enabled' => t('Enabled'),
    ),
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => 0,
    '#delta' => 50,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Add rule'));

  return $form;
}

function page_theme_admin_add_validate($form, &$form_state) {

  if (empty($form_state['values']['theme'])) {
    form_set_error('theme', t('Please select a theme.'));
  }
}

function page_theme_admin_add_submit($form, &$form_state) {

  config('page_theme.rule.' . $form_state['values']['rule'])
    ->setData(array(
      'rule' => $form_state['values']['rule'],
      'name' => $form_state['values']['name'],
      'theme' => $form_state['values']['theme'],
      'pages' => $form_state['values']['pages'],
      'roles' => array_keys(array_filter($form_state['values']['roles'])),
      'status' => intval(is_string($form_state['values']['status']['enabled'])),
      'weight' => $form_state['values']['weight'],
    ))
    ->save();

  backdrop_static_reset('page_theme_get_rules_enabled');
  menu_rebuild();

  backdrop_set_message(t('The rule has been added.'));
  $form_state['redirect'] = 'admin/appearance/page-theme';
}


/**
 * Menu callback; edits a rule.
 */
function page_theme_admin_edit($form, &$form_state, $rule) {

  backdrop_set_title(t('Configure rule %name', array('%name' => $rule['name'])), PASS_THROUGH);

  $form['old_rule'] = array(
      '#type' => 'value',
      '#value' => $rule['rule'],
  );
  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#description' => t('The human-readable name of this rule. It is recommended that this name begin with a capital letter and contain only letters, numbers, and spaces. This name must be unique.'),
    '#default_value' => $rule['name'],
    '#required' => TRUE,
    '#size' => 30,
  );
  $form['rule'] = array(
    '#type' => 'machine_name',
    '#description' => t('A unique machine-readable name for this rule. It must only contain lowercase letters, numbers, and underscores. This name will be used for constructing the URL of the %page-theme page, in which underscores will be converted into hyphens.', array('%page-theme' => t('Page theme'))),
    '#default_value' => $rule['rule'],
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'page_theme_rule_load',
    ),
  );
  $form['theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme'),
    '#description' => t('Choose which theme the page(s) should display in.'),
    '#default_value' => $rule['theme'],
    '#options' => page_theme_get_themes_options(),
    '#required' => TRUE,
  );
  $form['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#description' => t('Enter one path per line. The "*" character is a wildcard. Example paths are "node/1" for an individual piece of content or "node/*" for every piece of content. "@front" is the front page.', array('@front' => '<front>')),
    '#default_value' => $rule['pages'],
    '#required' => TRUE,
  );
  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles'),
    '#description' => t('Show this theme only for the selected role(s). If you select no roles, the theme will be visible to all users.'),
    '#default_value' => $rule['roles'],
    '#options' => array_map('check_plain', user_roles()),
  );
  $form['status'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Status'),
    '#default_value' => $rule['status'] ? array('enabled') : array(),
    '#options' => array(
      'enabled' => t('Enabled'),
    ),
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $rule['weight'],
    '#delta' => 50,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Update rule'));

  return $form;
}

function page_theme_admin_edit_validate($form, &$form_state) {

  if (empty($form_state['values']['theme'])) {
    form_set_error('theme', t('Please select a theme.'));
  }
}

function page_theme_admin_edit_submit($form, &$form_state) {

  config('page_theme.rule.' . $form_state['values']['rule'])
    ->setData(array(
      'rule' => $form_state['values']['rule'],
      'name' => $form_state['values']['name'],
      'theme' => $form_state['values']['theme'],
      'pages' => $form_state['values']['pages'],
      'roles' => array_keys(array_filter($form_state['values']['roles'])),
      'status' => intval(is_string($form_state['values']['status']['enabled'])),
      'weight' => $form_state['values']['weight'],
    ))
    ->save();

  // Delete the previous configuration file if any.
  if ($form_state['values']['rule'] != $form_state['values']['old_rule']) {
    config('page_theme.rule.' . $form_state['values']['old_rule'])
      ->delete();
  }

  backdrop_static_reset('page_theme_get_rules_enabled');
  menu_rebuild();

  backdrop_set_message(t('The rule has been updated.'));
  $form_state['redirect'] = 'admin/appearance/page-theme';
}


/**
 * Menu callback; deletes a rule.
 */
function page_theme_admin_delete($form, &$form_state, $rule) {

  $form['rule'] = array(
    '#type' => 'value',
    '#value' => $rule['rule'],
  );
  $question = t('Are you sure you want to delete the rule %name?', array('%name' => $rule['name']));
  $path = isset($_GET['destination']) ? $_GET['destination'] : 'admin/appearance/page-theme';

  return confirm_form($form, $question, $path);
}

function page_theme_admin_delete_submit($form, &$form_state) {

  if ($form_state['values']['confirm']) {

    config('page_theme.rule.' . $form_state['values']['rule'])
      ->delete();

    backdrop_static_reset('page_theme_get_rules_enabled');
    menu_rebuild();

    backdrop_set_message(t('The rule has been deleted.'));
    $form_state['redirect'] = 'admin/appearance/page-theme';
    return;
  }
}
