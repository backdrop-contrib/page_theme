<?php

/**
 * @file
 * Admin page callbacks for the page_theme module.
 */


/**
 * Menu callback; lists all defined themes.
 */
function page_theme_admin_overview($form, &$form_state) {

  $page_themes = page_theme_get_page_themes_all();

  $themes = array();

  foreach ($page_themes as $page_theme) {

    $row['name'] = array(
      '#markup' => page_theme_get_theme_name($page_theme['theme']),
    );
    $row['theme'] = array(
      '#type' => 'hidden',
      '#value' => $page_theme['theme'],
    );
    $row['pages_display'] = array(
      '#markup' => str_replace("\n", '<br>', check_plain($page_theme['pages'])),
    );
    $row['pages'] = array(
      '#type' => 'hidden',
      '#value' => $page_theme['pages'],
    );
    $row['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => $page_theme['status'],
    );
    $row['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $page_theme['weight'],
      '#delta' => 50,
      '#attributes' => array(
        'class' => array('page-theme-order-weight'),
      ),
    );
    $row['configure'] = array(
      '#markup' => l(t('configure'), 'admin/appearance/page-theme/manage/'. $page_theme['theme'] .'/configure'),
    );
    $row['delete'] = array(
      '#markup' => l(t('delete'), 'admin/appearance/page-theme/manage/'. $page_theme['theme'] .'/delete'),
    );

    $themes[$page_theme['theme']] = $row;
  }

  $form['themes'] = array(
    '#tree' => TRUE,
  );
  $form['themes'] += $themes;

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

function theme_page_theme_admin_overview($variables) {

  $form = $variables['form'];

  $rows = array();

  foreach (element_children($form['themes']) as $theme) {

    $row = array();
    $row[] = backdrop_render($form['themes'][$theme]['name']);
    $row[] = backdrop_render($form['themes'][$theme]['pages_display']);
    $row[] = backdrop_render($form['themes'][$theme]['status']);
    $row[] = backdrop_render($form['themes'][$theme]['weight']);
    $row[] = backdrop_render($form['themes'][$theme]['configure']);
    $row[] = backdrop_render($form['themes'][$theme]['delete']);

    $rows[] = array('data' => $row, 'class' => array('draggable'));
  }


  $output = '<p>' . t('If pages are several defined, the first theme in the list will be used.') . '</p>';
  $output .= '<p>' . t('Only themes, which are enabled in the <a href="@themes_section">themes section</a>, will be used otherwise the site default theme.', array('@themes_section' => url('admin/appearance'))) . '</p>';

  if (count($rows) > 0) {

    backdrop_add_tabledrag('page-theme-order', 'order', 'sibling', 'page-theme-order-weight');

    $header = array();
    $header[] = t('Theme');
    $header[] = t('Pages');
    $header[] = t('Enabled');
    $header[] = t('Weight');
    $header[] = array('data' => t('Operations'), 'colspan' => '2');

    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'page-theme-order')));
    $output .= backdrop_render_children($form);
  }
  else {

    $output .= '<p>' . backdrop_placeholder(t('No themes defined.')) . '</p>';
  }

  return $output;
}

function page_theme_admin_overview_submit($form, &$form_state) {

  foreach ($form_state['values']['themes'] as $theme => $value) {

    $field = array(
      'theme' => $value['theme'],
      'pages' => $value['pages'],
      'status' => $value['status'],
      'weight' => $value['weight'],
    );

    $config = config('page_theme.' . $field['theme']);
    $config->setData($field);
    $config->save();

    backdrop_static_reset('page_theme_get_page_themes_enabled');
  }

  backdrop_set_message(t('The configuration has been saved.'));
}


/**
 * Menu callback; adds a theme.
 */
function page_theme_admin_add($form, &$form_state) {

  $form['theme'] = array(
    '#type' => 'select',
    '#title' => t('Theme'),
    '#description' => t('Choose which theme the page should display in.'),
    '#default_value' => '0',
    '#options' => page_theme_get_theme_options(),
    '#required' => TRUE,
  );
  $form['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#description' => t('Enter one path per line. The "*" character is a wildcard. Example paths are "node/1" for an individual piece of content or "node/*" for every piece of content. "@front" is the front page.', array('@front' => '<front>')),
    '#default_value' => '',
    '#required' => TRUE,
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

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

function page_theme_admin_add_validate($form, &$form_state) {

  if (empty($form_state['values']['theme'])) {
    form_set_error('theme', t('Please select a theme.'));
  }
}

function page_theme_admin_add_submit($form, &$form_state) {

  $field = array(
    'theme' => $form_state['values']['theme'],
    'pages' => trim($form_state['values']['pages']),
    'status' => intval(is_string($form_state['values']['status']['enabled'])),
    'weight' => $form_state['values']['weight'],
  );

  $config = config('page_theme.' . $field['theme']);
  $config->setData($field);
  $config->save();

  backdrop_static_reset('page_theme_get_page_themes_enabled');

  backdrop_set_message(t('The theme has been added.'));
  $form_state['redirect'] = 'admin/appearance/page-theme';
}


/**
 * Menu callback; edits a theme.
 */
function page_theme_admin_edit($form, &$form_state, $page_theme) {

  $form['theme'] = array(
    '#type' => 'value',
    '#value' => $page_theme['theme'],
  );
  $form['theme_name'] = array(
    '#type' => 'item',
    '#title' => t('Theme'),
    '#markup' => page_theme_get_theme_name($page_theme['theme']),
  );
  $form['pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Pages'),
    '#description' => t('Enter one path per line. The "*" character is a wildcard. Example paths are "node/1" for an individual piece of content or "node/*" for every piece of content. "@front" is the front page.', array('@front' => '<front>')),
    '#default_value' => $page_theme['pages'],
    '#required' => TRUE,
  );
  $form['status'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Status'),
    '#default_value' => $page_theme['status'] ? array('enabled') : array(),
    '#options' => array(
      'enabled' => t('Enabled'),
    ),
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $page_theme['weight'],
    '#delta' => 50,
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration')
  );

  return $form;
}

function page_theme_admin_edit_submit($form, &$form_state) {

  $field = array(
    'theme' => $form_state['values']['theme'],
    'pages' => trim($form_state['values']['pages']),
    'status' => intval(is_string($form_state['values']['status']['enabled'])),
    'weight' => $form_state['values']['weight'],
  );

  $config = config('page_theme.' . $field['theme']);
  $config->setData($field);
  $config->save();

  backdrop_static_reset('page_theme_get_page_themes_enabled');

  backdrop_set_message(t('The theme has been updated.'));
  $form_state['redirect'] = 'admin/appearance/page-theme';
}


/**
 * Menu callback; deletes a theme.
 */
function page_theme_admin_delete($form, &$form_state, $page_theme) {

  $form['theme'] = array(
    '#type' => 'value',
    '#value' => $page_theme['theme'],
  );
  $question = t('Are you sure you want to delete the theme %theme?', array('%theme' => page_theme_get_themes($page_theme['theme'])));
  $path = isset($_GET['destination']) ? $_GET['destination'] : 'admin/appearance/page-theme';

  return confirm_form($form, $question, $path);
}

function page_theme_admin_delete_submit($form, &$form_state) {

  if ($form_state['values']['confirm']) {

    $theme = $form_state['values']['theme'];

    $config = config('page_theme.' . $theme);
    $config->delete();

    backdrop_static_reset('page_theme_get_page_themes_enabled');

    backdrop_set_message(t('The theme has been deleted.'));
    $form_state['redirect'] = 'admin/appearance/page-theme';
    return;
  }
}
