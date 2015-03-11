if exists("g:loaded_syntastic_glsl_driver_checker")
    finish
endif
let g:loaded_syntastic_glsl_driver_checker = 1

let s:save_cpo = &cpo
set cpo&vim

function! SyntaxCheckers_glsl_driver_GetLocList() dict
    let makeprg = self.makeprgBuild({
        \ 'args_before': s:GetShaderType() })

    let errorformat =
        \ "%E%f: 0(%l) : error %m," .
        \ "%W%f: 0(%l) : warning %m," .
        \ "%E%f: (0) : error %m," .
        \ "%W%f: (0) : warning %m"

    return SyntasticMake({
        \ 'makeprg': makeprg,
        \ 'errorformat': errorformat })
endfunction

function! s:GetShaderType()
    let save_view = winsaveview()
    let old_foldenable = &foldenable
    let old_lazyredraw = &lazyredraw

    let &lazyredraw = 1
    let &foldenable = 0
    call cursor(1, 1)

    let type_magic = '\m\C^// type:\s*'
    let sep_magic = '\m\C^// separable:\s*'
    let type_line = search(type_magic, 'c')
    let sep_line = search(sep_magic, 'c')

    call winrestview(save_view)
    let &foldenable = old_foldenable
    let &lazyredraw = old_lazyredraw

    if type_line
        let profile = matchstr(getline(type_line), type_magic . '\zs.*')
    else
        let profile = 'v'
    endif

    if sep_line
	let separable = matchstr( getline( sep_line ) , sep_magic . '\zs.*' )
    else
	let separable = 'no'
    endif
    let separable = ( separable ==? 'yes' ) ? '/ ' : ''

    return separable . profile
endfunction

call g:SyntasticRegistry.CreateAndRegisterChecker({
    \'filetype': 'glsl',
    \'name': 'driver',
    \'exec': 'check-glsl'})

let s:ownpath = expand( '<sfile>:p:h:h:h' )
let g:syntastic_glsl_driver_exec = s:ownpath . '/check-glsl'

let &cpo = s:save_cpo
unlet s:save_cpo

" vim: set sw=4 sts=4 et fdm=marker:

