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

    $row['rule'] = array(
      '#type' => 'value',
      '#value' => $rule['rule'],
    );
    $row['name'] = array(
      '#type' => 'value',
      '#value' => $rule['name'],
    );
    $row['name_display'] = array(
      '#markup' => check_plain($rule['name']) . '<br><small>' . t('(Machine name: @rule)', array('@rule' => $rule['rule'])) . '</small>',
    );
    $row['theme'] = array(
      '#type' => 'value',
      '#value' => $rule['theme'],
    );
    $row['theme_display'] = array(
      '#markup' => page_theme_get_theme_name($rule['theme'], TRUE),
    );
    $row['pages'] = array(
      '#type' => 'value',
      '#value' => $rule['pages'],
    );
    $row['pages_display'] = array(
      '#markup' => nl2br(check_plain($rule['pages'])),
    );
    $row['roles'] = array(
      '#type' => 'value',
      '#value' => $rule['roles'],
    );
    $row['roles_display'] = array(
      '#markup' => theme('item_list', array('items' => array_intersect_key(array_map('check_plain', user_roles()), array_flip($rule['roles'])))),
    );
    $row['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => $rule['status'],
    );
    $row['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $rule['weight'],
      '#delta' => 50,
      '#attributes' => array(
        'class' => array('page-theme-order-weight'),
      ),
    );
    $row['configure'] = array(
      '#markup' => l(t('configure'), 'admin/appearance/page-theme/manage/'. strtr($rule['rule'], array('_' => '-')) .'/configure'),
    );
    $row['delete'] = array(
      '#markup' => l(t('delete'), 'admin/appearance/page-theme/manage/'. strtr($rule['rule'], array('_' => '-')) .'/delete'),
    );

    $rules[$rule['rule']] = $row;
  }

  $form['rules'] = array('#tree' => TRUE);
  $form['rules'] += $rules;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function theme_page_theme_admin_overview($variables) {

  $form = $variables['form'];

  $rows = array();

  foreach (element_children($form['rules']) as $rule) {

    $row = array();
    $row[] = backdrop_render($form['rules'][$rule]['name_display']);
    $row[] = backdrop_render($form['rules'][$rule]['theme_display']);
    $row[] = backdrop_render($form['rules'][$rule]['pages_display']);
    $row[] = backdrop_render($form['rules'][$rule]['roles_display']);
    $row[] = backdrop_render($form['rules'][$rule]['status']);
    $row[] = backdrop_render($form['rules'][$rule]['weight']);
    $row[] = backdrop_render($form['rules'][$rule]['configure']);
    $row[] = backdrop_render($form['rules'][$rule]['delete']);

    $rows[] = array('data' => $row, 'class' => array('draggable'));
  }

  $header = array();
  $header[] = t('Name');
  $header[] = t('Theme');
  $header[] = t('Pages');
  $header[] = t('Roles');
  $header[] = t('Enabled');
  $header[] = t('Weight');
  $header[] = array('data' => t('Operations'), 'colspan' => '2');

  $output = '<p>' . t('If pages are several defined, the first rule in the list will be used.') . '</p>';
  $output .= '<p>' . t('Only themes, which are enabled in the <a href="@themes_section">themes section</a>, will be used otherwise the site default theme.', array('@themes_section' => url('admin/appearance'))) . '</p>';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'page-theme-order'),
    'empty' => backdrop_placeholder(t('No rules defined.')),
  ));
  $output .= backdrop_render_children($form);

  backdrop_add_tabledrag('page-theme-order', 'order', 'sibling', 'page-theme-order-weight');

  return $output;
}

function page_theme_admin_overview_submit($form, &$form_state) {

  foreach ($form_state['values']['rules'] as $rule) {

    config('page_theme.rule.' . $rule['rule'])
      ->setData(array(
        'rule' => $rule['rule'],
        'name' => $rule['name'],
        'theme' => $rule['theme'],
        'pages' => $rule['pages'],
        'roles' => $rule['roles'],
        'status' => $rule['status'],
        'weight' => $rule['weight'],
      ))
      ->save();

    backdrop_static_reset('page_theme_get_rules_enabled');
  }

  backdrop_set_message(t('The configuration has been saved.'));
}


/**
 * Menu callback; adds a rule.
 */
function page_theme_admin_add($form, &$form_state) {

  backdrop_set_title(t('Add rule'));

  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
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
    '#description' => t('Show this theme only for the selected role(s). If you select no roles, the block will be visible to all users.'),
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
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

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

  backdrop_set_message(t('The rule has been added.'));
  $form_state['redirect'] = 'admin/appearance/page-theme';
}


/**
 * Menu callback; edits a rule.
 */
function page_theme_admin_edit($form, &$form_state, $rule) {

  $form['obsolete'] = array(
      '#type' => 'value',
      '#value' => $rule['rule'],
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
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
    '#description' => t('Show this theme only for the selected role(s). If you select no roles, the block will be visible to all users.'),
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
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function page_theme_admin_edit_validate($form, &$form_state) {

  if (empty($form_state['values']['theme'])) {
    form_set_error('theme', t('Please select a theme.'));
  }
}

function page_theme_admin_edit_submit($form, &$form_state) {

  config('page_theme.rule.' . $form_state['values']['obsolete'])
      ->delete();

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

    backdrop_set_message(t('The rule has been deleted.'));
    $form_state['redirect'] = 'admin/appearance/page-theme';
    return;
  }
}
